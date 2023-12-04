#include <Arduino.h>
#include <EEPROM.h>
#include <esp_task_wdt.h>
#include <vector>
#include "Punch.h"
#include <driver/gpio.h>

#define u8 uint8_t
// Pin attatch
namespace pin {
constexpr auto VALVE_ADD      = GPIO_NUM_27;
constexpr auto VALVE_DECREASE = GPIO_NUM_14;
constexpr auto PUNCH          = GPIO_NUM_25;
constexpr auto LED            = GPIO_NUM_2;
}
constexpr auto EEP_size = 30;

Punch punch;
std::vector<short *> RS_MAP_DATA;
hw_timer_t *timer1 = nullptr;
bool timer_started = false;

// 0:punch_delay  1:punch_out 2:punch_stay 3:poll_back
uint8_t punch_step_STA = 0;
uint8_t save_STA       = 0;
//-----timer1 callback
void timer1_callback() {
  timerStop(timer1);
  timer_started = false;
  punch_step_STA++;
  if (punch_step_STA == 4) {
    punch_step_STA = 0;
  }
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

void setup() {
  // put your setup code here, to run once:
  EEPROM.begin(EEP_size);
  pinMode(pin::VALVE_ADD, OUTPUT); // val
  pinMode(pin::VALVE_DECREASE, OUTPUT);
  pinMode(pin::PUNCH, INPUT);
  Serial.begin(115200);
  Serial2.begin(115200);
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
}

int last_feed_time = 0, key_last = 0;

void loop() {
  if (digitalRead(pin::PUNCH) == 1 && timer_started) {
    switch (punch_step_STA) {
      case 0:
        Serial.println("punch_delay");
        punch_delay();
        break;
      case 1:
        Serial.println("punch_out ");
        punch_out();
        break;
      case 2:
        Serial.println("punch_stay ");
        punch_stay();
        break;
      case 3:
        Serial.println("poll_back ");
        poll_back();
    }
  }
  if (digitalRead(pin::PUNCH) == 0) {
    if (millis() - key_last > 5) {
      punch_step_STA = 0;
      timerStop(timer1);
      timer_started = false;
      timerWrite(timer1, 0);
    }
  } else {
    key_last = millis();
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
}

#ifndef PLATFORMIO
extern "C" [[noreturn]] void app_main(void) {
  setup();
  while (true) {
    loop();
  }
}
#endif
