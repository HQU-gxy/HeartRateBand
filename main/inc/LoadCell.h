#ifndef PUNCHER_LOAD_CELL_H
#define PUNCHER_LOAD_CELL_H
#include <Arduino.h>
#include <HX711.h>
#include "common.h"
class LoadCell {
private:
  gpio_num_t DOUT;
  gpio_num_t PD_SCK;
  HX711 Hx711{};
  static constexpr uint8_t buf_size = 30;

public:
  LoadCell(gpio_num_t d_out, gpio_num_t pd_sck) : DOUT(d_out), PD_SCK(pd_sck) {}

  uint8_t measure_cnt          = 0;
  static constexpr auto per_kg = 21000;
  float punch_power            = 0;
  float measure_buf[buf_size]{};

  bool begin() {
    Hx711.begin(DOUT, PD_SCK);
    Hx711.wait_ready_retry(10, 200);
    if (Hx711.is_ready()) {
      Hx711.tare(10);
    } else {
      return false;
    }
    return true;
  }

  void measure_once() {
    float temp = Hx711.get_units(1);
    if (temp > per_kg) {
      punch_power              = temp / per_kg;
      measure_buf[measure_cnt] = punch_power;
      measure_cnt++;
    }
    if (measure_cnt == buf_size) {
      measure_cnt = 0;
    }
  }

  void measure_n(uint8_t n) {
    measure_buf[measure_cnt] = Hx711.get_units(3);
    measure_cnt++;
    if (measure_cnt == buf_size) {
      measure_cnt = 0;
      // TODO
    }
  }
};

#endif