//
// Created by Kurosu Chan on 2023/12/7.
//
#include <esp_log.h>
#include "value_reading.h"

namespace protocol {

etl::expected<request_t, CborError> decode_command(const uint8_t *buffer, size_t size) {
  constexpr static auto TAG = "decode_command";
  using ue_t                = etl::unexpected<CborError>;
  CborError err;
  CborParser parser;
  CborValue value;
  err = cbor_parser_init(buffer, size, 0, &parser, &value);
  if (err != CborNoError) {
    return ue_t{err};
  }
  if (!cbor_value_is_array(&value)) {
    return ue_t{CborErrorIllegalType};
  }

  CborValue array_elem;
  err = cbor_value_enter_container(&value, &array_elem);
  if (err != CborNoError) {
    return ue_t{err};
  }

  if (cbor_value_get_type(&array_elem) != CborIntegerType) {
    return ue_t{CborErrorIllegalType};
  }

  int magic = 0;
  err       = cbor_value_get_int(&array_elem, &magic);
  if (err != CborNoError) {
    return ue_t{err};
  }

  switch (magic) {
    case static_cast<uint8_t>(Command::ONCE):
      return request_t{Command::ONCE};
    case static_cast<uint8_t>(Command::SUCCESSIVE):
      return request_t{Command::SUCCESSIVE};
    case static_cast<uint8_t>(Command::STOP):
      return request_t{Command::STOP};
    case static_cast<uint8_t>(Command::TARE):
      return request_t{Command::TARE};
    case static_cast<uint8_t>(Command::BTN_DISABLE):
      return request_t{Command::BTN_DISABLE};
    case static_cast<uint8_t>(Command::BTN_ENABLE):
      return request_t{Command::BTN_ENABLE};
    case static_cast<uint8_t>(change_duration_t::MAGIC): {
      err = cbor_value_advance(&array_elem);
      if (err != CborNoError) {
        return ue_t{err};
      }
      if (!cbor_value_is_integer(&array_elem)) {
        ESP_LOGE(TAG, "illegal type for array element; expected integer");
        return ue_t{CborErrorIllegalType};
      }
      int duration = 0;
      err          = cbor_value_get_int(&array_elem, &duration);
      if (err != CborNoError) {
        ESP_LOGE(TAG, "cbor_value_get_int: %s", cbor_error_string(err));
        return ue_t{err};
      }
      return request_t{change_duration_t{duration}};
    }
    default:
      return ue_t{CborUnknownError};
  }
}
}
