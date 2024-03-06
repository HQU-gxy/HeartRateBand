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
#include <esp_task_wdt.h>

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
extern "C" void app_main(void) {
  constexpr auto TAG = "main";
  initArduino();

  auto config = esp_task_wdt_config_t{
      .timeout_ms    = 3000,
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

  // value, time_diff
  using pair_t = std::tuple<float, uint16_t>;
  static etl::deque<pair_t, PUNCH_MEASUREMENT_COUNT> values;

  static auto sensor       = peripheral::LoadCell{pin::D_OUT, pin::DP_SCK};
  static auto valve        = peripheral::Valve{pin::VALVE_ADD, pin::VALVE_DECREASE};
  static auto punch_switch = peripheral::Switch{pin::PUNCH_BTN};
  punch_switch.en          = true;
restart:
  pinMode(pin::LED, OUTPUT);
  digitalWrite(pin::LED, HIGH);

  punch_switch.on_close = []() {
    valve.idle();
  };
  punch_switch.on_open = []() {
    valve.successive();
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
    // goto restart;
  }

  valve.begin();
  punch_switch.begin();

  struct timer_param_t {
    std::function<void()> callback;
    TaskHandle_t handle;
  };

  auto send_msg = []() {
    constexpr auto TAG    = "send_msg";
    constexpr auto buf_sz = 256;
    uint8_t buf[buf_sz];
    if (values.empty()) {
      return;
    }

    const auto dbg_str = [](const decltype(values) &values) {
      std::string str;
      str += "[";
      auto sz = 0;
      for (const auto &v : values) {
        str += "(";
        str += std::to_string(std::get<0>(v));
        str += ", ";
        str += std::to_string(std::get<1>(v));
        str += ")";
        sz++;
        if (sz < values.size()) {
          str += ", ";
        }
      }
      str += "]";
      return str;
    };

    auto dbg = dbg_str(values);
    ESP_LOGI(TAG, "values: %s", dbg.c_str());
    auto res = protocol::encode_load_cell_reading(values.begin(), values.end(), buf, buf_sz);
    if (res.has_value()) {
      const auto sz = res.value();
      ESP_LOGI(TAG, "encoded %d bytes; reading size %d;", sz, values.size());
      auto msg = wlan::MqttPubMsg{
          .topic = "/puncher/reading",
          .data  = std::vector<uint8_t>(buf, buf + sz),
      };
      manager.publish(msg);
      values.clear();
    } else {
      ESP_LOGE(TAG, "encode failed: %d", res.error());
    }
  };

  static auto timer = timer_param_t{
      .callback = send_msg,
  };

  auto timer_cb = [](TimerHandle_t xTimer) {
    auto p      = pvTimerGetTimerID(xTimer);
    auto &param = *static_cast<timer_param_t *>(p);
    auto fn     = [](void *pvParameters) {
      auto &param = *static_cast<timer_param_t *>(pvParameters);
      param.callback();
      auto handle  = param.handle;
      param.handle = nullptr;
      vTaskDelete(handle);
    };
    xTaskCreate(fn,
                "send_msg",
                4096,
                &param,
                1,
                &param.handle);
  };

  static TimerHandle_t timer_handle = xTimerCreate("send_msg",
                                                   common::PUNCH_MEASUREMENT_SEND_INTERVAL_MS / portTICK_PERIOD_MS,
                                                   pdTRUE,
                                                   &timer,
                                                   timer_cb);
  xTimerStart(timer_handle, portMAX_DELAY);

  static auto callbacks = handler::callbacks_t{
      .on_once            = []() { valve.once(); },
      .on_successive      = []() { valve.successive(); },
      .on_stop            = []() { valve.idle(); },
      .on_tare            = []() { sensor.tare(); },
      .on_switch_disable  = []() {
        ESP_LOGI(TAG, "switch disable");
        punch_switch.en = false; },
      .on_switch_enable   = []() {
        ESP_LOGI(TAG, "switch enable");
        punch_switch.en = true; },
      .on_change_duration = [](int duration_ms) {
        if (duration_ms < 100) {
          ESP_LOGE(TAG, "duration too short: %d", duration_ms);
          return;
        }
        if (duration_ms > 3000) {
          ESP_LOGE(TAG, "duration too long: %d", duration_ms);
          return;
        }
        valve.set_delay(peripheral::PunchStep::ReachingOut, std::chrono::milliseconds(duration_ms));
        ESP_LOGI(TAG, "change Reaching out duration: %d", duration_ms); },
  };

  static auto handler_params = handler::param_t{
      .callbacks    = std::move(callbacks),
      .sub_msg_chan = *manager.sub_msg_chan(),
  };

  static TaskHandle_t handler_handle;
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

  static etl::optional<decltype(millis())> last_measurement = etl::nullopt;
  const auto loop                                                 = []{
    constexpr auto TAG = "loop";
    punch_switch.poll();
    valve.poll();
    sensor.measure();
    const auto val = sensor.take_average();
    if (val.has_value()) {
      uint16_t diff;
      if (last_measurement.has_value()) {
        auto diff_ = millis() - last_measurement.value();
        if (diff_ > 0xFFFF) {
          diff = 0xFFFF;
        } else {
          diff = static_cast<uint16_t>(diff_);
        }
      } else {
        diff = 0;
      }
      enqueue_value(values, std::make_tuple(val.value(), diff), PUNCH_MEASUREMENT_COUNT);
      last_measurement = millis();
    }
  };

  struct loop_param_t {
    std::function<void()> callback;
    TaskHandle_t handle;
  };

  static auto loop_param = loop_param_t{
      .callback = loop,
  };

  auto loop_task = [](void *pvParameters) {
    auto &param = *static_cast<loop_param_t *>(pvParameters);
    while (true) {
      param.callback();
      ESP_ERROR_CHECK(esp_task_wdt_reset());
    }
    std::unreachable();
  };

  xTaskCreate(loop_task,
              "loop",
              4096,
              &loop_param,
              1,
              &loop_param.handle);

  ESP_ERROR_CHECK(esp_task_wdt_add(loop_param.handle));
  vTaskDelete(nullptr);
}
