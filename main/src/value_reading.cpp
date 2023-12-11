//
// Created by Kurosu Chan on 2023/12/7.
//
#include <esp_log.h>
#include "value_reading.h"

namespace protocol {

etl::expected<Command, CborError> decode_command(const uint8_t *buffer, size_t size) {
  constexpr static auto TAG = "decode_command";
  using ue_t                = etl::unexpected<CborError>;
  CborError err;
  CborParser parser;
  CborValue value;
  int magic = 0;
  err       = cbor_parser_init(buffer, size, 0, &parser, &value);
  if (err != CborNoError) {
    return ue_t{err};
  }

  // A lambda to encapsulate the common operation
  auto get_int_value = [](CborValue *value) -> etl::expected<int, CborError> {
    CborError err;
    auto t    = cbor_value_get_type(value);
    int magic = 0;
    switch (t) {
      case CborIntegerType:
        err = cbor_value_get_int(value, &magic);
        break;
      case CborSimpleType:
        err = cbor_value_get_simple_type(value, reinterpret_cast<uint8_t *>(&magic));
        break;
      default:
        return ue_t{CborUnknownError};
    }
    if (err != CborNoError) {
      return ue_t{err};
    }
    return {magic};
  };

  auto t = cbor_value_get_type(&value);
  switch (t) {
    case CborArrayType: {
      err = cbor_value_enter_container(&value, &value);
      if (err != CborNoError) {
        return ue_t{err};
      }
      auto res = get_int_value(&value);
      if (!res.has_value()) {
        return ue_t{res.error()};
      }
      magic = res.value();
      break;
    }
    case CborIntegerType:
    case CborSimpleType: {
      auto res = get_int_value(&value);
      if (!res.has_value()) {
        return ue_t{res.error()};
      }
      magic = res.value();
      break;
    }
    default:
      return ue_t{CborUnknownError};
      break;
  }
  switch (magic) {
    case static_cast<uint8_t>(Command::ONCE):
      return Command::ONCE;
    case static_cast<uint8_t>(Command::SUCCESSIVE):
      return Command::SUCCESSIVE;
    case static_cast<uint8_t>(Command::STOP):
      return Command::STOP;
    case static_cast<uint8_t>(Command::TARE):
      return Command::TARE;
    case static_cast<uint8_t>(Command::BTN_DISABLE):
      return Command::BTN_DISABLE;
    case static_cast<uint8_t>(Command::BTN_ENABLE):
      return Command::BTN_ENABLE;
    default:
      return ue_t{CborUnknownError};
  }
}
}
