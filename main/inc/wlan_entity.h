//
// Created by Kurosu Chan on 2023/10/19.
//

#ifndef TRACK_HUB_WLAN_ENTITY_H
#define TRACK_HUB_WLAN_ENTITY_H

#include <msd/channel.hpp>
#include <string>

namespace wlan {
const auto BROKER_URL = "mqtt://weihua-iot.cn:1883";
struct MqttPubMsg {
  std::string topic;
  std::vector<uint8_t> data;
  int qos = 0;
  // https://www.hivemq.com/blog/mqtt-essentials-part-8-retained-messages/
  int retain = 0;
};

struct MqttSubMsg {
  std::string topic;
  std::vector<uint8_t> data;
};

using sub_msg_chan_t = msd::channel<MqttSubMsg>;
}

#endif // TRACK_HUB_WLAN_ENTITY_H
