//
// Created by Kurosu Chan on 2023/12/4.
//

#ifndef WIT_HUB_COMMON_H
#define WIT_HUB_COMMON_H

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

constexpr auto DEFAULT_DURATION                     = std::chrono::milliseconds(1000);
constexpr auto LOAD_CELL_BUF_SIZE                   = 30;
static constexpr auto LOAD_CELL_DEFAULT_UNIT_PER_KG = 21000;
constexpr auto MAX_SUB_TOPIC_COUNT                  = 8;

static constexpr auto ChanBit = BIT2;
}

#endif // WIT_HUB_COMMON_H
