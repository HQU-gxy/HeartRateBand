//
// Created by Kurosu Chan on 2023/12/4.
//

#ifndef PUNCHER_INSTANT_H
#define PUNCHER_INSTANT_H

#include <etl/delegate.h>
#include <chrono>

/**
 * @brief A counter that counts the time since the program starts
 * @tparam T the type of the counter
 * @sa https://doc.rust-lang.org/std/time/struct.Instant.html
 */
template <std::unsigned_integral T = size_t>
class Instant {
  T time_;

public:
  using time_t     = decltype(time_);
  using duration_t = std::chrono::duration<time_t, std::milli>;

  /**
   * @brief Get the time since the program starts in milliseconds
   * @return the time since the program starts in milliseconds
   */
  static time_t millis() {
    // esp_timer_get_time() returns the time since boot in microseconds
    return static_cast<time_t>(::esp_timer_get_time() / 1000);
  }

  Instant() {
    this->time_ = millis();
  }

  static Instant now() {
    return Instant{};
  }

  [[nodiscard]] duration_t elapsed() const {
    const auto now = static_cast<time_t>(millis());
    if (now < this->time_) {
      // overflow
      return duration_t{now + (std::numeric_limits<time_t>::max() - this->time_)};
    }
    return duration_t{now - this->time_};
  }

  [[nodiscard]] time_t elapsed_ms() const {
    return static_cast<time_t>(this->elapsed().count());
  }

  [[nodiscard]] bool has_elapsed_ms(const time_t ms) const {
    return this->elapsed_ms() >= ms;
  }

  /**
   * @brief Checks if the specified time interval has elapsed since the last reset.
   *
   * This method checks if the elapsed time since the last reset is greater than or equal to the input parameter `ms`.
   * If it is, the method resets the internal timer and returns true. If not, it simply returns false.
   * The `[[nodiscard]]` attribute indicates that the compiler will issue a warning if the return value of this function is not used.
   *
   * @param ms The time interval in milliseconds.
   * @return Returns true if the elapsed time is greater than or equal to the input time interval, otherwise returns false.
   */
  [[nodiscard]] bool every_ms(const time_t ms) {
    const bool gt = this->elapsed_ms() >= ms;
    if (gt) {
      this->reset();
      return true;
    }
    return false;
  }

  template <typename Rep, typename Period>
  [[nodiscard]] bool has_elapsed(const std::chrono::duration<Rep, Period> duration) const {
    return this->elapsed() >= duration;
  }

  template <typename Rep, typename Period>
  [[nodiscard]] bool every(const std::chrono::duration<Rep, Period> duration) {
    const bool gt = this->has_elapsed(duration);
    if (gt) {
      this->reset();
      return true;
    }
    return false;
  }

  void reset() {
    this->time_ = millis();
  }

  [[nodiscard]] duration_t elapsed_and_reset() {
    const auto now = static_cast<time_t>(millis());
    decltype(now) diff;
    if (now < this->time_) {
      // overflow
      diff = now + (std::numeric_limits<time_t>::max() - this->time_);
    } else {
      diff = now - this->time_;
    }
    const auto duration = duration_t(diff);
    this->time_         = now;
    return duration;
  }

  [[nodiscard]] time_t count() const {
    return time_;
  }
};

#endif // PUNCHER_INSTANT_H
