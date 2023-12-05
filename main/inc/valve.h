//
// Created by Kurosu Chan on 2023/12/4.
//

#ifndef PUNCHER_VALVE_H
#define PUNCHER_VALVE_H

#include "common.h"

namespace peripheral {
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
  using tp_t                = Instant::timepoint_t;
  using duration_t          = std::chrono::milliseconds;

private:
  gpio_num_t add_;
  gpio_num_t decrease_;
  PunchStep state                                   = PunchStep::Delay;
  etl::flat_map<PunchStep, duration_t, 4> delay_map = {
      {PunchStep::Delay, common::DEFAULT_DURATION},
      {PunchStep::Out, common::DEFAULT_DURATION},
      {PunchStep::Stay, common::DEFAULT_DURATION},
      {PunchStep::Back, common::DEFAULT_DURATION},
  };
  tp_t disable_time_point = 0;
  bool is_enabled_        = false;
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
  Valve(gpio_num_t add, gpio_num_t decrease) : add_(add), decrease_(decrease) {
    disable_time_point = esp_timer_get_time();
  }

  void begin() {
    pinMode(add_, OUTPUT);
    pinMode(decrease_, OUTPUT);
  }

  void set_delay(PunchStep step, duration_t delay) {
    delay_map[step] = delay;
  }

  [[nodiscard]] bool is_enabled() const {
    return is_enabled_;
  }

  void enable() {
    is_enabled_  = true;
    auto now     = esp_timer_get_time();
    auto elapsed = now - disable_time_point;
    instant.add(elapsed);
  }

  void disable() {
    is_enabled_        = false;
    disable_time_point = esp_timer_get_time();
  }

  void poll() {
    bool run = instant.elapsed() > delay_map[state];
    if (run && is_enabled_) {
      next_action();
      ESP_LOGI(TAG, "state %s", to_string(state).c_str());
      instant.reset();
    }
  }
};
}

#endif // PUNCHER_VALVE_H
