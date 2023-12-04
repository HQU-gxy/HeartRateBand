#include <Arduino.h>
#include "instant.h"
#include "LoadCell.h"
#include "common.h"
#include <etl/flat_map.h>
#include <button.h>
#include <driver/gpio.h>

using namespace common;

// https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/timer.html#timeralarm
using tp_t       = decltype(millis());
using duration_t = std::chrono::milliseconds;
enum class PunchStep {
  Delay = 0,
  Out,
  Stay,
  Back,
};

std::string to_string(PunchStep step) {
  switch (step) {
    case PunchStep::Delay:
      return "Delay";
    case PunchStep::Out:
      return "Out";
    case PunchStep::Stay:
      return "Stay";
    case PunchStep::Back:
      return "Back";
    default:
      std::unreachable();
  }
}

struct ValveOut {
  bool add;
  bool decrease;
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
      return PunchStep::Delay;
    default:
      std::unreachable();
  }
}

ValveOut valve(PunchStep step) {
  switch (step) {
    case PunchStep::Delay:
      return {0, 0};
    case PunchStep::Out:
      return {1, 0};
    case PunchStep::Stay:
      return {0, 0};
    case PunchStep::Back:
      return {0, 1};
    default:
      std::unreachable();
  }
}

class Valve {
public:
  static constexpr auto TAG = "Valve";

private:
  gpio_num_t add_;
  gpio_num_t decrease_;
  PunchStep state                                   = PunchStep::Delay;
  etl::flat_map<PunchStep, duration_t, 4> delay_map = {
      {PunchStep::Delay, DEFAULT_DURATION},
      {PunchStep::Out, DEFAULT_DURATION},
      {PunchStep::Stay, DEFAULT_DURATION},
      {PunchStep::Back, DEFAULT_DURATION},
  };
  Instant instant;

  void action(PunchStep step) {
    auto [add, decrease] = valve(step);
    gpio_set_level(add_, add);
    gpio_set_level(decrease_, decrease);
  }

  void next_state() {
    state = next(state);
  }

  void next_action() {
    next_state();
    action(state);
  }

public:
  Valve(gpio_num_t add, gpio_num_t decrease) : add_(add), decrease_(decrease) {}

  void begin() {
    pinMode(add_, OUTPUT);
    pinMode(decrease_, OUTPUT);
  }

  void set_delay(PunchStep step, duration_t delay) {
    delay_map[step] = delay;
  }

  void reset_instant() {
    instant.reset();
  }

  void poll() {
    bool run = instant.elapsed() > delay_map[state];
    if (run) {
      next_action();
      ESP_LOGI(TAG, "state %s", to_string(state).c_str());
      instant.reset();
    }
  }
};

extern "C" [[noreturn]] void app_main(void) {
  initArduino();
  static auto sensor    = LoadCell{pin::D_OUT, pin::DP_SCK};
  static auto valve     = Valve{pin::VALVE_ADD, pin::VALVE_DECREASE};
  static auto punch_btn = peripheral::EdgeButton{pin::PUNCH_BTN};
  pinMode(pin::LED, OUTPUT);
  digitalWrite(pin::LED, HIGH);
  using BtnState = peripheral::ButtonState;

  punch_btn.on_release = []() {
    valve.reset_instant();
  };
  sensor.begin();
  valve.begin();
  punch_btn.begin();

  auto loop = []() {
    constexpr auto TAG = "loop";

    punch_btn.poll();
    if (punch_btn.state() == BtnState ::Pressed) {
      valve.poll();
    } else {
      // nothing
    }
    sensor.measure_once();
  };
  while (true) {
    loop();
  }
}
