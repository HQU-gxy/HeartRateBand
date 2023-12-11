//
// Created by Kurosu Chan on 2023/12/7.
//
#include "event_handler.h"
#include "value_reading.h"
#include <esp_log.h>
void handler::handle(void *pvParameters) {
  auto param                = static_cast<param_t *>(pvParameters);
  auto &callbacks           = param->callbacks;
  auto &sub_msg_chan        = param->sub_msg_chan;
  static constexpr auto TAG = "command";
  for (auto msg : sub_msg_chan) {
    constexpr auto command_topic = "command";
    if (msg.topic.find(command_topic) != std::string::npos) {
      auto res      = protocol::decode_command(msg.data.data(), msg.data.size());
      using Command = protocol::Command;
      if (res.has_value()) {
        auto command = res.value();
        switch (command) {
          case Command::ONCE:
            ESP_LOGI(TAG, "once");
            callbacks.on_once();
            break;
          case Command::SUCCESSIVE:
            ESP_LOGI(TAG, "successive");
            callbacks.on_successive();
            break;
          case Command::STOP:
            ESP_LOGI(TAG, "stop");
            callbacks.on_stop();
            break;
          case Command::TARE:
            ESP_LOGI(TAG, "tare");
            callbacks.on_tare();
            break;
          case Command::BTN_DISABLE:
            ESP_LOGI(TAG, "btn_disable");
            callbacks.on_switch_disable();
            break;
          case Command::BTN_ENABLE:
            ESP_LOGI(TAG, "btn_enable");
            callbacks.on_switch_enable();
            break;
          default:
            std::unreachable();
        }
      } else {
        ESP_LOGE(TAG, "decode command: %s", cbor_error_string(res.error()));
      }
    }
  }
  std::unreachable();
}
