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
 * @class Button
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
class Button {
public:
  explicit Button(gpio_num_t pin) : pin_(pin) {}
  std::function<void()> on_press;
  std::function<void()> on_release;

private:
  gpio_num_t pin_;
  ButtonState state                   = ButtonState::Released;
  ButtonState last_executed_state     = ButtonState::Released;
  constexpr static auto debounce_time = std::chrono::milliseconds(30);
  Instant instant;

  void execute() const {
    if (state == ButtonState::Pressed) {
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

  void poll() {
    auto new_state = digitalRead(pin_) > 0 ? ButtonState::Pressed : ButtonState::Released;

    if (new_state != state) {
      instant.reset();
      state = new_state;
    }

    if (instant.elapsed() > debounce_time && state != last_executed_state) {
      execute();
      last_executed_state = state;
    }
  }
};
}

#endif // PUNCHER_BUTTON_H
