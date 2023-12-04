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
  constexpr auto PUNCH          = GPIO_NUM_25;
  constexpr auto LED            = GPIO_NUM_2;
  constexpr auto D_OUT          = GPIO_NUM_34;
  constexpr auto DP_SCK         = GPIO_NUM_14;
}
}

#endif // WIT_HUB_COMMON_H
