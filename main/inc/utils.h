//
// Created by Kurosu Chan on 2023/12/5.
//

#ifndef PUNCHER_UTILS_H
#define PUNCHER_UTILS_H

#include <etl/optional.h>

#ifdef htons
#undef htons
#endif

#ifdef ntohs
#undef ntohs
#endif

#ifdef htonl
#undef htonl
#endif

#ifdef ntohl
#undef ntohl
#endif

namespace utils {
// https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
inline uint16_t htons(uint16_t val) {
  if constexpr (std::endian::native == std::endian::little) {
#if defined __has_builtin
#if __has_builtin(__builtin_bswap16)
#define HAS_BUILTIN_BSWAP16
#endif
#endif

#ifdef HAS_BUILTIN_BSWAP16
    return __builtin_bswap16(val);
#else
#warning "no builtin bswap16, use naive implementation"
    return (val << 8) | (val >> 8);
#endif
  } else {
    return val;
  }
}

inline uint16_t ntohs(uint16_t val) {
  if constexpr (std::endian::native == std::endian::little) {
#if defined __has_builtin
#if __has_builtin(__builtin_bswap16)
#define HAS_BUILTIN_BSWAP16
#endif
#endif

#ifdef HAS_BUILTIN_BSWAP16
    return __builtin_bswap16(val);
#else
#warning "no builtin bswap16, use naive implementation"
    return (val << 8) | (val >> 8);
#endif
  } else {
    return val;
  }
}

inline uint32_t htonl(uint32_t val) {
  if constexpr (std::endian::native == std::endian::little) {
#if defined __has_builtin
#if __has_builtin(__builtin_bswap32)
#define HAS_BUILTIN_BSWAP32
#endif
#endif

#ifdef HAS_BUILTIN_BSWAP32
    return __builtin_bswap32(val);
#else
#warning "no builtin bswap32, use naive implementation"
    return (val << 24) | ((val << 8) & 0x00ff0000) | ((val >> 8) & 0x0000ff00) | (val >> 24);
#endif
  } else {
    return val;
  }
}

inline uint32_t ntohl(uint32_t val) {
  if constexpr (std::endian::native == std::endian::little) {
#if defined __has_builtin
#if __has_builtin(__builtin_bswap32)
#define HAS_BUILTIN_BSWAP32
#endif
#endif

#ifdef HAS_BUILTIN_BSWAP32
    return __builtin_bswap32(val);
#else
#warning "no builtin bswap32, use naive implementation"
    return (val << 24) | ((val << 8) & 0x00ff0000) | ((val >> 8) & 0x0000ff00) | (val >> 24);
#endif
  } else {
    return val;
  }
}

size_t sprint_hex(char *out, size_t outSize, const uint8_t *bytes, size_t size);

std::string to_hex(const uint8_t *bytes, size_t size);

template <size_t N>
class SimpleMovingAverage {
private:
  float sum   = 0;
  size_t size = 0;
  etl::optional<float> value_;

public:
  SimpleMovingAverage() = default;
  float next(const float value) {
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

  size_t get_size() {
    return size;
  }

  void reset() {
    sum  = 0;
    size = 0;
    value_.reset();
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

// helper constant for the visitor #3
template <class>
inline constexpr bool always_false_v = false;

// helper type for the visitor #4
template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
}

static constexpr auto htons = utils::htons;
static constexpr auto ntohs = utils::ntohs;
static constexpr auto htonl = utils::htonl;
static constexpr auto ntohl = utils::ntohl;

#endif // PUNCHER_UTILS_H
