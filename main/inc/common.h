//
// Created by Kurosu Chan on 2023/12/4.
//

#ifndef WIT_HUB_COMMON_H
#define WIT_HUB_COMMON_H

#include <chrono>
#include <driver/gpio.h>

namespace common {
namespace pin {
  constexpr auto VALVE_ADD      = GPIO_NUM_27;
  constexpr auto VALVE_DECREASE = GPIO_NUM_14;
  constexpr auto PUNCH_BTN      = GPIO_NUM_25;
  constexpr auto LED            = GPIO_NUM_2;
  constexpr auto D_OUT          = GPIO_NUM_34;
  constexpr auto DP_SCK         = GPIO_NUM_14;
}

namespace nvs {
  constexpr auto PARTITION      = "st";
  constexpr auto PUNCH_STEP_KEY = "pst";
  using punch_step_t            = uint8_t;
}

constexpr auto DEFAULT_DURATION  = std::chrono::milliseconds(1000);
constexpr auto LOAD_CELL_MA_SIZE = 10;
/**
 * @brief the coefficient to convert the load cell reading
 *
 * the actual reading should be divided by this value and
 * the receiver should multiply the value by this value.
 * The purpose of this is mainly to reduce the size of the
 * data to be sent, to make it fall into the range of half
 * precision floating point (float16).
 *
 * To convert to kg, divide the original reading by around 20500.
 * The exact value is to be measured.
 */
static constexpr auto LOAD_CELL_COEF              = 10000;
static constexpr auto LOAD_CELL_THRES             = 10000 / LOAD_CELL_COEF;
constexpr auto MAX_SUB_TOPIC_COUNT                = 8;
constexpr auto PUNCH_MEASUREMENT_COUNT            = 50;
constexpr auto SAMPLE_INTERVAL_MS                 = 12;
constexpr auto PUNCH_MEASUREMENT_SEND_INTERVAL_MS = 500;

static constexpr auto ChanBit = BIT2;
}

#endif // WIT_HUB_COMMON_H
