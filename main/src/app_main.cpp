#include <Arduino.h>
#include "instant.h"
#include "LoadCell.h"
#include "common.h"
#include <etl/flat_map.h>
#include "button.h"
#include "valve.h"
#include "wlan_manager.h"
#include "utils.h"
#include "app_nvs.h"
#include "value_reading.h"
#include <freertos/timers.h>
#include "event_handler.h"
#include <driver/gpio.h>

#define stringify_literal(x)     #x
#define stringify_expanded(x)    stringify_literal(x)
#define stringify_with_quotes(x) stringify_expanded(stringify_expanded(x))

#ifndef WLAN_AP_SSID
#define WLAN_AP_SSID default
#endif

#ifndef WLAN_AP_PASSWORD
#define WLAN_AP_PASSWORD default
#endif

// https://learn.microsoft.com/en-us/cpp/preprocessor/stringizing-operator-hash?view=msvc-170
const char *WLAN_SSID     = stringify_expanded(WLAN_AP_SSID);
const char *WLAN_PASSWORD = stringify_expanded(WLAN_AP_PASSWORD);

using namespace common;

template <typename T>
void enqueue_value(etl::ideque<T> &values, T value, size_t max_size) {
  auto sz = values.size();
  while (sz >= max_size) {
    values.pop_front();
    sz = values.size();
  }
  values.push_back(value);
}

// https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/timer.html#timeralarm
extern "C" [[noreturn]] void app_main(void) {
  constexpr auto TAG = "main";
  initArduino();

  auto ap = wlan::AP{WLAN_SSID, WLAN_PASSWORD};
  ESP_LOGI(TAG, "ssid=%s; password=%s;", ap.ssid.c_str(), ap.password.c_str());
  static auto evt_grp = xEventGroupCreate();
  static auto manager = wlan::WlanManager(evt_grp);
  manager.set_ap(std::move(ap));
  ESP_ERROR_CHECK(manager.wifi_init());
  ESP_ERROR_CHECK(manager.start_connect_task());
  ESP_ERROR_CHECK(manager.mqtt_init());

  static etl::deque<float, PUNCH_MEASUREMENT_COUNT> values;

  static auto sensor    = peripheral::LoadCell{pin::D_OUT, pin::DP_SCK};
  static auto valve     = peripheral::Valve{pin::VALVE_ADD, pin::VALVE_DECREASE};
  static auto punch_btn = peripheral::EdgeButton{pin::PUNCH_BTN};
restart:
  pinMode(pin::LED, OUTPUT);
  digitalWrite(pin::LED, HIGH);

  punch_btn.on_press   = []() {};
  punch_btn.on_release = []() {
    valve.once();
  };

  auto last_step = app_nvs::get_punch_step();
  if (last_step.has_value()) {
    ESP_LOGI(TAG, "last valve step=%d", last_step.value());
    valve.set_step(static_cast<peripheral::PunchStep>(last_step.value()));
  }

  valve.on_step_change = [](peripheral::PunchStep step) {
    app_nvs::set_punch_step(static_cast<common::nvs::punch_step_t>(step));
  };

  bool ok = sensor.begin();
  if (!ok) {
    ESP_LOGE(TAG, "sensor begin failed");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    goto restart;
  }

  valve.begin();
  punch_btn.begin();

  struct timer_param_t {
    std::function<void()> callback;
  };

  auto send_msg = []() {
    constexpr auto TAG    = "send_msg";
    constexpr auto buf_sz = 256;
    uint8_t buf[buf_sz];
    auto res = protocol::encode_load_cell_reading(values.begin(), values.end(), buf, buf_sz);
    if (res.has_value()) {
      const auto sz = res.value();
      ESP_LOGI(TAG, "encoded %d bytes", sz);
      auto msg = wlan::MqttPubMsg{
          .topic = "/puncher/reading",
          .data  = std::vector<uint8_t>(buf, buf + sz),
      };
      manager.publish(msg);
    } else {
      ESP_LOGE(TAG, "encode failed: %d", res.error());
    }
  };

  static auto timer = timer_param_t{
      .callback = send_msg,
  };

  auto timer_cb = [](TimerHandle_t xTimer) {
    auto param = pvTimerGetTimerID(xTimer);
    auto &p    = *static_cast<timer_param_t *>(param);
    p.callback();
  };

  TimerHandle_t timer_handle = xTimerCreate("send_msg",
                                            common::PUNCH_MEASUREMENT_SEND_INTERVAL_MS / portTICK_PERIOD_MS,
                                            pdTRUE,
                                            &timer,
                                            timer_cb);
  xTimerStart(timer_handle, portMAX_DELAY);

  auto callbacks = handler::callbacks_t{
      .on_once       = []() { valve.once(); },
      .on_successive = []() { valve.successive(); },
      .on_stop       = []() { valve.idle(); },
      .on_tare       = []() { sensor.tare(); },
  };

  static auto handler_params = handler::param_t{
      .callbacks    = std::move(callbacks),
      .sub_msg_chan = *manager.sub_msg_chan(),
  };

  TaskHandle_t handler_handle;
  auto res = xTaskCreate(handler::handle,
                         "handler",
                         4096,
                         &handler_params,
                         1,
                         &handler_handle);

  if (res != pdPASS) {
    ESP_LOGE(TAG, "create handler task failed");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    goto restart;
  }

  auto loop = []() {
    constexpr auto TAG = "loop";
    punch_btn.poll();
    valve.poll();
    sensor.measure();
    auto val = sensor.average();
    if (val.has_value()) {
      enqueue_value(values, val.value(), PUNCH_MEASUREMENT_COUNT);
    }
  };

  while (true) {
    loop();
  }
}
