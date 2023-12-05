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
class MovingAverage {
private:
  float sum   = 0;
  size_t size = 0;
  etl::optional<float> value_;

public:
  MovingAverage() = default;
  float next(float value) {
    if (size < N) {
      sum += value;
      size++;
      value_ = sum / size;
      return *value_;
    } else {
      sum -= sum / size;
      sum += value;
      value_ = sum / size;
      return *value_;
    }
  }

  etl::optional<float> get() {
    return value_;
  }
};

}

#endif // PUNCHER_UTILS_H
