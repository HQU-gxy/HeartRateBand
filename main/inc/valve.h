//
// Created by Kurosu Chan on 2023/12/4.
//

#ifndef PUNCHER_VALVE_H
#define PUNCHER_VALVE_H

#include "common.h"

namespace peripheral {
enum class PunchStep {
  Retracted = 0,
  ReachingOut,
  Extended,
  PullingBack,
};

enum class PunchState {
  Idle = 0,
  Successive,
  Once,
};

std::string to_string(PunchStep step) {
  switch (step) {
    case PunchStep::Retracted:
      return "Retracted";
    case PunchStep::ReachingOut:
      return "ReachingOut";
    case PunchStep::Extended:
      return "Extended";
    case PunchStep::PullingBack:
      return "PullingBack";
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
    case PunchStep::Retracted:
      return PunchStep::ReachingOut;
    case PunchStep::ReachingOut:
      return PunchStep::Extended;
    case PunchStep::Extended:
      return PunchStep::PullingBack;
    case PunchStep::PullingBack:
      return PunchStep::Retracted;
    default:
      std::unreachable();
  }
}

ValveOut to_io(PunchStep step) {
  switch (step) {
    case PunchStep::Retracted:
      return {0, 0};
    case PunchStep::ReachingOut:
      return {1, 0};
    case PunchStep::Extended:
      return {0, 0};
    case PunchStep::PullingBack:
      return {0, 1};
    default:
      std::unreachable();
  }
}

class Valve {
public:
  static constexpr auto TAG                     = "Valve";
  using tp_t                                    = Instant::timepoint_t;
  using duration_t                              = std::chrono::milliseconds;
  std::function<void(PunchStep)> on_step_change = nullptr;

private:
  gpio_num_t add_;
  gpio_num_t decrease_;
  PunchStep step_   = PunchStep::Retracted;
  PunchState state_ = PunchState::Idle;

  etl::flat_map<PunchStep, duration_t, 4> delay_map = {
      {PunchStep::Retracted, common::DEFAULT_DURATION},
      {PunchStep::ReachingOut, common::DEFAULT_DURATION},
      {PunchStep::Extended, common::DEFAULT_DURATION},
      {PunchStep::PullingBack, common::DEFAULT_DURATION},
  };

  tp_t disable_time_point = 0;
  Instant instant;

  void action(PunchStep step) {
    auto [add, decrease] = to_io(step);
    gpio_set_level(add_, add);
    gpio_set_level(decrease_, decrease);
  }

  void on_successive() {
    auto step = next(step_);
    action(step);
    step_ = step;
    if (on_step_change != nullptr) {
      on_step_change(step_);
    }
    instant.reset();
  }

  void on_once() {
    auto step = next(step_);
    action(step);
    step_ = step;
    if (on_step_change != nullptr) {
      on_step_change(step_);
    }
    instant.reset();
    if (step_ == PunchStep::Retracted) {
      idle();
    }
  }

public:
  Valve(gpio_num_t add, gpio_num_t decrease) : add_(add), decrease_(decrease) {
    disable_time_point = esp_timer_get_time();
  }

  void begin() {
    pinMode(add_, OUTPUT);
    pinMode(decrease_, OUTPUT);
  }

  /**
   * @brief Set the step object
   * @warning usually user should not change the step manually.
   * It should only be used for providing the initial step.
   */
  void set_step(PunchStep step) {
    step_ = step;
  }

  void set_delay(PunchStep step, duration_t delay) {
    delay_map[step] = delay;
  }

  [[nodiscard]] bool is_active() const {
    return state_ != PunchState::Idle;
  }

  void successive() {
    state_       = PunchState::Successive;
    auto now     = esp_timer_get_time();
    auto elapsed = now - disable_time_point;
    instant.add(elapsed);
  }

  void once() {
    state_       = PunchState::Once;
    auto now     = esp_timer_get_time();
    auto elapsed = now - disable_time_point;
    instant.add(elapsed);
  }

  void idle() {
    state_             = PunchState::Idle;
    disable_time_point = esp_timer_get_time();
  }

  void poll() {
    if (state_ == PunchState::Idle) {
      return;
    }
    bool run = instant.elapsed() > delay_map[step_];
    if (run) {
      if (state_ == PunchState::Successive) {
        on_successive();
      } else if (state_ == PunchState::Once) {
        on_once();
      }
      ESP_LOGI(TAG, "step=%s", to_string(step_).c_str());
    }
  }
};
}

#endif // PUNCHER_VALVE_H
