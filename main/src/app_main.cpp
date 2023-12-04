#include <Arduino.h>
#include <EEPROM.h>
#include <esp_task_wdt.h>
#include "Punch.h"
#include "common.h"
#include <driver/gpio.h>

using namespace common;
hw_timer_t *timer1 = nullptr;

using tp_t = decltype(millis());
enum class PunchStep {
  Delay = 0,
  Out,
  Stay,
  Back,
};

PunchStep next(PunchStep step) {
  switch (step) {
    case PunchStep::Delay:
      return PunchStep::Out;
    case PunchStep::Out:
      return PunchStep::Stay;
    case PunchStep::Stay:
      return PunchStep::Back;
    case PunchStep::Back:
    default:
      return PunchStep::Delay;
  }
}

PunchStep punch_step_STA = PunchStep::Delay;
uint8_t save_STA         = 0;
tp_t last_feed_time      = 0;
tp_t key_last_tp_ms      = 0;
bool timer_started       = false;

void timer1_callback() {
  timerStop(timer1);
  timer_started  = false;
  punch_step_STA = next(punch_step_STA);
}

constexpr uint16_t punch_out_time = 1;
constexpr uint16_t poll_back_time = 1;
constexpr uint16_t punch_delay_ms = 1;
constexpr uint16_t punch_out_stay = 1;
void punch_out() {
  digitalWrite(pin::VALVE_ADD, 1);
  digitalWrite(pin::VALVE_DECREASE, 0);
  timerAlarm(timer1, punch_out_time * 1000, true, 0);
  timerStart(timer1);
  timer_started = true;
}

void poll_back() {
  digitalWrite(pin::VALVE_ADD, 0);
  digitalWrite(pin::VALVE_DECREASE, 1);
  timerAlarm(timer1, poll_back_time * 1000, true, 0);
  timerStart(timer1);
  timer_started = true;
}

void punch_stay() {
  digitalWrite(pin::VALVE_ADD, 0);
  digitalWrite(pin::VALVE_DECREASE, 0);
  timerAlarm(timer1, punch_out_stay * 1000, true, 0);
  timerStart(timer1);
  timer_started = true;
}

void punch_delay() {
  digitalWrite(pin::VALVE_ADD, 0);
  digitalWrite(pin::VALVE_DECREASE, 0);
  timerAlarm(timer1, punch_delay_ms * 1000, true, 0);
  timerStart(timer1);
  timer_started = true;
}

extern "C" [[noreturn]] void app_main(void) {
  initArduino();
  static auto punch = Punch{pin::D_OUT, pin::DP_SCK};
  pinMode(pin::VALVE_ADD, OUTPUT); // val
  pinMode(pin::VALVE_DECREASE, OUTPUT);
  pinMode(pin::PUNCH, INPUT);
  pinMode(pin::LED, OUTPUT);
  digitalWrite(pin::LED, 1);
  auto freq              = timerGetFrequency(timer1);
  constexpr auto divider = 80;

  timer1 = timerBegin(freq / 80); // timer1 -> valve
  timerAttachInterrupt(timer1, &timer1_callback);
  // https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/timer.html#timeralarm
  timerAlarm(timer1, 1000, true, 0);
  // timerAlarmEnable(timer1);
  timerStop(timer1);
  timer_started = false;
  timerWrite(timer1, 0);
  delay(1000);
  punch.punch_init();

  auto loop = []() {
    constexpr auto TAG = "loop";
    if (digitalRead(pin::PUNCH) == 1 && timer_started) {
      switch (punch_step_STA) {
        case PunchStep::Delay:
          ESP_LOGI(TAG, "punch_delay");
          punch_delay();
          break;
        case PunchStep::Out:
          ESP_LOGI(TAG, "punch_out");
          punch_out();
          break;
        case PunchStep::Stay:
          ESP_LOGI(TAG, "punch_stay");
          punch_stay();
          break;
        case PunchStep::Back:
          ESP_LOGI(TAG, "poll_back");
          poll_back();
      }
    }
    if (digitalRead(pin::PUNCH) == 0) {
      if (millis() - key_last_tp_ms > 5) {
        punch_step_STA = PunchStep::Delay;
        timerStop(timer1);
        timer_started = false;
        timerWrite(timer1, 0);
      }
    } else {
      key_last_tp_ms = millis();
    }

    if (save_STA) {
      save_STA = 0;
    }

    punch.measure_once_upper();

    if ((millis() - last_feed_time) > 2000) {
      esp_task_wdt_add(nullptr);
      esp_task_wdt_reset();
      last_feed_time = millis();
    }
  };
  while (true) {
    loop();
  }
}
