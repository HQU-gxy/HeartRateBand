#include <Arduino.h>
#include "instant.h"
#include "LoadCell.h"
#include "common.h"
#include <etl/flat_map.h>
#include "button.h"
#include "valve.h"
#include "utils.h"
#include <driver/gpio.h>

using namespace common;

// https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/timer.html#timeralarm
extern "C" [[noreturn]] void app_main(void) {
  constexpr auto TAG = "main";
  initArduino();
  static auto sensor    = LoadCell{pin::D_OUT, pin::DP_SCK};
  static auto valve     = peripheral::Valve{pin::VALVE_ADD, pin::VALVE_DECREASE};
  static auto punch_btn = peripheral::EdgeButton{pin::PUNCH_BTN};
restart:
  pinMode(pin::LED, OUTPUT);
  digitalWrite(pin::LED, HIGH);

  punch_btn.on_press   = []() {};
  punch_btn.on_release = []() {
    if (valve.is_enabled()) {
      valve.disable();
    } else {
      valve.enable();
    }
  };

  bool ok = sensor.begin();
  if (!ok) {
    ESP_LOGE(TAG, "sensor begin failed");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    goto restart;
  }
  valve.begin();
  punch_btn.begin();

  auto loop = []() {
    constexpr auto TAG = "loop";

    punch_btn.poll();
    valve.poll();
    sensor.measure();
  };

  while (true) {
    loop();
  }
}
