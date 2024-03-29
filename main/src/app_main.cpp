#include "undef_arduino.h"
#include "instant.h"
#include "common.h"
#include <etl/flat_map.h>
#include "wlan_manager.h"
#include <lwip/sockets.h>
#include <freertos/timers.h>
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <esp_task_wdt.h>
#include <MAX30102.h>
#include <cbor.h>
#include <etl/vector.h>
#include <etl/span.h>
#include <etl/expected.h>
#include <cib/cib.hpp>
#include "utils.h"

#define stringify_literal(x)     #x
#define stringify_expanded(x)    stringify_literal(x)
#define stringify_with_quotes(x) stringify_expanded(stringify_expanded(x))

#ifndef WLAN_AP_SSID
#define WLAN_AP_SSID default
#endif

#ifndef WLAN_AP_PASSWORD
#define WLAN_AP_PASSWORD default
#endif

constexpr auto HOST_IP       = "192.168.2.228";
constexpr uint16_t HOST_PORT = 8080;

using namespace common;

const char *WLAN_SSID     = stringify_expanded(WLAN_AP_SSID);
const char *WLAN_PASSWORD = stringify_expanded(WLAN_AP_PASSWORD);

#define CBOR_RETURN_WHEN_ERROR(expr)    \
  do {                                  \
    CborError err = (expr);             \
    if (err != CborNoError) return err; \
  } while (0)

#define CBOR_RETURN_UE_WHEN_ERROR(expr)                             \
  do {                                                              \
    CborError err = (expr);                                         \
    if (err != CborNoError) return etl::unexpected<CborError>{err}; \
  } while (0)

// https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_client
static constexpr auto i2c_task = [] {
  static MAX30102 sensor{};
restart:
  bool ok  = sensor.begin();
  bool ok_ = sensor.setSamplingRate(sensor.SAMPLING_RATE_400SPS);
  if (not(ok or ok_)) {
    ESP_LOGE("i2c_task", "MAX30105 not found or setSamplingRate failed. Please check the wiring");
    vTaskDelay(pdMS_TO_TICKS(1000));
    goto restart;
  }

  static constexpr auto BUFFER_COUNT = 200;
  constexpr auto BUFFER_SIZE         = ((BUFFER_COUNT + 10) * sizeof(uint32_t) * 2);
  static size_t item_count           = 0;

  static etl::array<uint8_t, BUFFER_SIZE> buffer{};

  static CborEncoder encoder{};
  static CborEncoder array_encoder{};

  // https://intel.github.io/tinycbor/current/a00046.html
  constexpr auto re_init = []() -> CborError {
    item_count = 0;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    CBOR_RETURN_WHEN_ERROR(cbor_encoder_create_array(&encoder, &array_encoder, BUFFER_COUNT));

    return CborNoError;
  };

  sensor.setMode(sensor.MODE_RED_IR);
  sensor.setADCRange(sensor.ADC_RANGE_8192NA);
  sensor.setResolution(sensor.RESOLUTION_17BIT_215US);
  const auto err = re_init();
  if (err != CborNoError) {
    ESP_LOGE("i2c_task", "re_init cbor buffer failed: %s (%d)", cbor_error_string(err), err);
    vTaskDelay(pdMS_TO_TICKS(1000));
    goto restart;
  }

  // ******* UDP *******

  using socketaddr_in_t = struct sockaddr_in;
  using timeval_t       = struct timeval;

  auto dest_addr            = socketaddr_in_t{};
  dest_addr.sin_addr.s_addr = inet_addr(HOST_IP);
  dest_addr.sin_family      = AF_INET;
  dest_addr.sin_port        = htons(HOST_PORT);
  const auto sock           = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock < 0) {
    ESP_LOGE("i2c_task", "Unable to create socket: errno %d", errno);
    vTaskDelay(pdMS_TO_TICKS(1000));
    goto restart;
  }

  auto timeout   = timeval_t{};
  timeout.tv_sec = 5;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  ESP_LOGI("i2c_task", "Socket created, sending to %s:%d", HOST_IP, HOST_PORT);

  const auto loop = [dest_addr, sock, &re_init] {
    const auto to_down_stream = [dest_addr, sock](const etl::span<uint8_t> buf) {
      auto addr           = dest_addr;
      const auto addr_ptr = reinterpret_cast<sockaddr *>(&addr);
      const auto err      = sendto(sock, buf.data(), buf.size(), 0, addr_ptr, sizeof(dest_addr));
      if (err < 0) {
        ESP_LOGE("i2c_task", "Error occurred during sending: errno %d", errno);
      }
    };
    const auto sample = sensor.readSample(100);
    if (not sample.valid) {
      return;
    }
    // const auto red_err = cbor_encode_uint(&red_encoder, sample.red);
    // const auto ir_err  = cbor_encode_uint(&ir_encoder, sample.ir);
    const auto add_elem = [](uint32_t red, uint32_t ir) -> etl::expected<etl::monostate, CborError> {
      CborEncoder tuple_encoder{};
      CBOR_RETURN_UE_WHEN_ERROR(cbor_encoder_create_array(&array_encoder, &tuple_encoder, 2));
      CBOR_RETURN_UE_WHEN_ERROR(cbor_encode_uint(&tuple_encoder, red));
      CBOR_RETURN_UE_WHEN_ERROR(cbor_encode_uint(&tuple_encoder, ir));
      CBOR_RETURN_UE_WHEN_ERROR(cbor_encoder_close_container(&array_encoder, &tuple_encoder));
      return {};
    };
    const auto err = add_elem(sample.red, sample.ir);
    if (not err) {
      ESP_LOGE("i2c_task", "add_elem failed: %s (%d)", cbor_error_string(err.error()), err.error());
      re_init();
      return;
    }
    item_count += 1;
    const auto is_full = item_count >= BUFFER_COUNT;
    if (is_full) {
      static constexpr auto close_and_get_size = [] -> etl::expected<size_t, CborError> {
        CBOR_RETURN_UE_WHEN_ERROR(cbor_encoder_close_container(&encoder, &array_encoder));
        const auto len = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return len;
      };
      const auto len_ = close_and_get_size();
      if (not len_) {
        ESP_LOGE("i2c_task", "close_and_get_size failed: %s (%d)", cbor_error_string(len_.error()), len_.error());
        re_init();
        return;
      }
      const auto len = len_.value();
      ESP_LOGI("i2c_task", "cbor buffer size: %d", len);
      auto down = etl::span{buffer.data(), len};
      to_down_stream(down);
      re_init();
    }
  };

  for (;;) {
    loop();
  }
};

// https://github.com/espressif/esp-idf/issues/12098
// https://github.com/KJ7LNW/esp32-i2c-test/blob/d6383e7d1f815feb44d06685b7f3d16caa7c844f/main/i2c-test.c#L126
extern "C" void app_main(void) {
  constexpr auto TAG = "main";
  initArduino();

  const auto config = esp_task_wdt_config_t{
      .timeout_ms    = 5000,
      .trigger_panic = true,
  };
  ESP_ERROR_CHECK(esp_task_wdt_init(&config));

  auto ap = wlan::AP{WLAN_SSID, WLAN_PASSWORD};
  ESP_LOGI(TAG, "ssid=%s; password=%s;", ap.ssid.c_str(), ap.password.c_str());
  static auto evt_grp = xEventGroupCreate();
  static auto manager = wlan::WlanManager(evt_grp);
  manager.set_ap(ap);
  ESP_ERROR_CHECK(manager.wifi_init());
  ESP_ERROR_CHECK(manager.start_connect_task());
  // ESP_ERROR_CHECK(manager.mqtt_init());
  i2c_task();
  vTaskDelete(nullptr);
}
