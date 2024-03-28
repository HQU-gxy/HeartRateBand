#include "undef_arduino.h"
#include "instant.h"
#include "common.h"
#include <etl/flat_map.h>
#include "wlan_manager.h"
#include "utils.h"
#include <freertos/timers.h>
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <esp_task_wdt.h>
#include <MAX30102.h>
#include <etl/vector.h>
#include <cib/cib.hpp>

#define stringify_literal(x)     #x
#define stringify_expanded(x)    stringify_literal(x)
#define stringify_with_quotes(x) stringify_expanded(stringify_expanded(x))

#ifndef WLAN_AP_SSID
#define WLAN_AP_SSID default
#endif

#ifndef WLAN_AP_PASSWORD
#define WLAN_AP_PASSWORD default
#endif

const char *WLAN_SSID     = stringify_expanded(WLAN_AP_SSID);
const char *WLAN_PASSWORD = stringify_expanded(WLAN_AP_PASSWORD);

using namespace common;

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
  constexpr auto BUFFER_SIZE = 200;
  // TODO: use COBS
  // it just a array of uint32_t as buffer
  static etl::vector<uint32_t, BUFFER_SIZE> red_buffer{};
  static etl::vector<uint32_t, BUFFER_SIZE> ir_buffer{};
  sensor.setMode(sensor.MODE_RED_IR);
  sensor.setADCRange(sensor.ADC_RANGE_8192NA);
  sensor.setResolution(sensor.RESOLUTION_17BIT_215US);
  for (;;) {
    const auto sample = sensor.readSample(1000);
    if (not sample.valid) {
      continue;
    }
    red_buffer.push_back(sample.red);
    ir_buffer.push_back(sample.ir);
    // remember a IR reading at unblocked state (when the sensor is not attached to a finger/arm)
    // if delta large than threshold then something is blocking the sensor
    if (red_buffer.size() == BUFFER_SIZE) {
      // to UDP
      // What's the sample rate again?
      red_buffer.clear();
      ir_buffer.clear();
    }
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
  manager.set_ap(std::move(ap));
  ESP_ERROR_CHECK(manager.wifi_init());
  ESP_ERROR_CHECK(manager.start_connect_task());
  ESP_ERROR_CHECK(manager.mqtt_init());
  vTaskDelete(nullptr);
}
