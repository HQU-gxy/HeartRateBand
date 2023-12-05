//
// Created by Kurosu Chan on 2023/12/5.
//

#ifndef PUNCHER_UTILS_H
#define PUNCHER_UTILS_H

#include <etl/optional.h>

namespace utils {

size_t sprintHex(char *out, size_t outSize, const uint8_t *bytes, size_t size);

std::string toHex(const uint8_t *bytes, size_t size);

template <size_t N>
class SimpleMovingAverage {
private:
  float sum   = 0;
  size_t size = 0;
  etl::optional<float> value_;

public:
  SimpleMovingAverage() = default;
  float next(float value) {
    if (size < N) {
      sum += value;
      size++;
      value_ = sum / size;
    } else {
      sum -= sum / N;
      sum += value;
      value_ = sum / N;
    }
    return *value_;
  }

  etl::optional<float> get() {
    return value_;
  }
};

template <size_t N>
class ExponentMovingAverage {
private:
  etl::optional<float> value_;

public:
  ExponentMovingAverage() = default;
  float next(float value) {
    if (!value_.has_value()) {
      value_ = value;
    } else {
      constexpr float k = 2.0f / (N + 1);
      static_assert(k < 1.0f);
      static_assert(k > 0.0f);
      auto old = *value_;
      value_   = (value * k) + (old * (1 - k));
    }
    return *value_;
  }

  etl::optional<float> get() {
    return value_;
  }
};

}

#endif // PUNCHER_UTILS_H
