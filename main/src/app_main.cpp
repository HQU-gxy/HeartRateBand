#include <Arduino.h>
#include "instant.h"
#include "common.h"
#include <etl/flat_map.h>
#include "wlan_manager.h"
#include "utils.h"
#include <freertos/timers.h>
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
  vTaskDelete(nullptr);
}
