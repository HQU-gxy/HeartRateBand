//
// Created by Kurosu Chan on 2023/12/4.
//

#ifndef PUNCHER_BUTTON_H
#define PUNCHER_BUTTON_H

#include <Arduino.h>
#include <driver/gpio.h>

namespace peripheral {
enum class ButtonState {
  Pressed,
  Released,
};

/**
 * @class EdgeButton
 *
 * Represents a digital button connected to a GPIO pin.
 * This class provides debouncing functionality to filter out quick,
 * undesired state changes (noise). It also allows the assignment
 * of custom functions to be called when the button is pressed or released.
 *
 * The `on_press` and `on_release` functions are not called continuously
 * while the button remains in one state. They are only called
 * once for each press or release event.
 *
 * @warning The `poll()` method should be called frequently in order to accurately
 * detect button press and release events.
 * As such, avoid executing long-running tasks in the `on_press` and `on_release`
 * callbacks as this could delay the execution of `poll()`
 * and potentially cause button presses or releases to be missed.
 * Even worse, blocking the whole event loop.
 */
class EdgeButton {
public:
  explicit EdgeButton(gpio_num_t pin) : pin_(pin) {}
  /**
   * @brief The duration that the button must remain in the same state
   */
  std::chrono::milliseconds debounce_duration = std::chrono::milliseconds(50);
  std::function<void()> on_press;
  std::function<void()> on_release;

private:
  gpio_num_t pin_;
  ButtonState state_               = ButtonState::Released;
  ButtonState last_executed_state_ = ButtonState::Released;
  Instant instant;

  void execute() const {
    if (state_ == ButtonState::Pressed) {
      if (on_press) {
        on_press();
      }
    } else {
      if (on_release) {
        on_release();
      }
    }
  }

public:
  void begin() {
    pinMode(pin_, INPUT);
  }

  /**
   * @brief Returns the current state of the button
   * @note This is the debounced state that is updated by `poll()`
   */
  [[nodiscard]] ButtonState state() const {
    return last_executed_state_;
  }

  void poll() {
    auto new_state = digitalRead(pin_) > 0 ? ButtonState::Pressed : ButtonState::Released;

    if (new_state != state_) {
      instant.reset();
      state_ = new_state;
    }

    if (instant.elapsed() > debounce_duration && state_ != last_executed_state_) {
      execute();
      last_executed_state_ = state_;
    }
  }
};

class Switch {
private:
  gpio_num_t pin_;

public:
  explicit Switch(gpio_num_t pin) : pin_(pin) {}
  /**
   * @brief Whether the switch is enabled
   *
   * If disabled, the switch won't trigger any callbacks
   */
  bool en = true;
  std::function<void()> on_open;
  std::function<void()> on_close;

  void begin() {
    pinMode(pin_, INPUT);
  }

  void poll() {
    if (!en) {
      return;
    }
    if (digitalRead(pin_) > 0) {
      if (on_open) {
        on_open();
      }
    } else {
      if (on_close) {
        on_close();
      }
    }
  }
};

}

#endif // PUNCHER_BUTTON_H
