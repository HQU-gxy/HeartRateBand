//
// Created by Kurosu Chan on 2023/12/7.
//

#ifndef PUNCHER_EVENT_HANDLER_H
#define PUNCHER_EVENT_HANDLER_H

#include <functional>
#include <wlan_entity.h>

namespace handler {
struct callbacks_t {
  std::function<void()> on_once;
  std::function<void()> on_successive;
  std::function<void()> on_stop;
  std::function<void()> on_tare;
};

struct param_t {
  callbacks_t callbacks;
  wlan::sub_msg_chan_t &sub_msg_chan;
};

[[noreturn]] void handle(void *pvParameters);
;

}

#endif // PUNCHER_EVENT_HANDLER_H
