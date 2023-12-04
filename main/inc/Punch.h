#ifndef _PUNCH_H
#define _PUNCH_H
#include <Arduino.h>
#include <HX711.h>
#include "common.h"
class Punch {
private:
  gpio_num_t DOUT;
  gpio_num_t PD_SCK;
  HX711 Hx711{};
  static constexpr uint8_t buf_size = 30;

public:
  Punch(gpio_num_t d_out, gpio_num_t pd_sck) : DOUT(d_out), PD_SCK(pd_sck) {}

  uint8_t measure_cnt = 0;
  int per_kg          = 21000;
  float punch_power   = 0;
  float measure_buf[buf_size]{};

  bool punch_init() {
    Hx711.begin(DOUT, PD_SCK);
    Hx711.wait_ready_retry(10, 200);
    if (Hx711.is_ready()) {
      Hx711.tare(10);
    } else {
      return false;
    }
    return true;
  }

  void measure_once_upper() {
    float temp = Hx711.get_units(1);
    if (temp > this->per_kg) {
      punch_power              = temp / this->per_kg;
      measure_buf[measure_cnt] = punch_power;
      measure_cnt++;
    }
    if (measure_cnt == buf_size) {
      measure_cnt = 0;
    }
  }
  void measure_times(uint8_t times) {
    measure_buf[measure_cnt] = Hx711.get_units(3);
    measure_cnt++;
    if (measure_cnt == buf_size) {
      measure_cnt = 0;
      this->send_buf(times);
    }
  }
  void send_buf(uint8_t times) {
    float sum_temp     = 0;
    float average_temp = 0;
    for (auto i = 0; i < times; i++) {
      sum_temp += measure_buf[i];
    }
  }
  void set_per_kg(int kg) {
    if (kg < 40000 && kg > 10000) {
      this->per_kg = kg;
    }
  }
};

#endif