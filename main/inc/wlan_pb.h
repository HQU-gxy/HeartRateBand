//
// Created by Kurosu Chan on 2023/10/18.
//

#ifndef TRACK_HUB_WLAN_PB_H
#define TRACK_HUB_WLAN_PB_H

#include <string>
#include <functional>
#include <etl/optional.h>
#include <variant>

namespace wlan {
struct AP {
  std::string ssid;
  std::string password;
};

using ssid_fn     = std::function<bool(std::string)>;
using password_fn = std::function<bool(std::string)>;
}

#endif // TRACK_HUB_WLAN_PB_H
