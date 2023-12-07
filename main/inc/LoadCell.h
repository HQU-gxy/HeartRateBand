#ifndef PUNCHER_LOAD_CELL_H
#define PUNCHER_LOAD_CELL_H
#include <Arduino.h>
#include <HX711.h>
#include <etl/deque.h>
#include <etl/optional.h>
#include "utils.h"
#include "common.h"

namespace peripheral {
class LoadCell {
private:
  gpio_num_t DOUT;
  gpio_num_t PD_SCK;
  HX711 Hx711{};

  utils::ExponentMovingAverage<common::LOAD_CELL_MA_SIZE> buf;
  /**
   * @brief acts like a cell, each value can only be taken once
   */
  bool to_be_taken = false;

public:
  LoadCell(gpio_num_t d_out, gpio_num_t pd_sck) : DOUT(d_out), PD_SCK(pd_sck) {}

  bool begin() {
    Hx711.begin(DOUT, PD_SCK);
    Hx711.wait_ready_retry(10, 200);
    if (Hx711.is_ready()) {
      // "Tare" is a term used in weighing that refers to the weight of a
      // container or wrapper that is holding the material to be weighed.
      // When you tare a scale, you're setting the scale back to zero,
      // but you're doing so to account for the weight of the container
      // that's holding what you're actually interested in weighing.
      //
      // read the average value of N samples
      // and set the average as offset
      Hx711.tare(10);
    } else {
      return false;
    }
    return true;
  }

  void tare() {
    Hx711.tare(10);
  }

  void measure(uint8_t n = 1) {
    if (!Hx711.is_ready()) {
      return;
    }
    float temp = Hx711.get_units(n);
    buf.next(temp / common::LOAD_CELL_COEF);
    to_be_taken = true;
  }

  etl::optional<float> average() {
    auto temp = buf.get();
    if (temp > static_cast<float>(common::LOAD_CELL_THRES)) {
      return temp;
    }
    return etl::nullopt;
  }

  etl::optional<float> take_average() {
    if (to_be_taken) {
      to_be_taken = false;
      return average();
    }
    return etl::nullopt;
  }
};
}

#endif