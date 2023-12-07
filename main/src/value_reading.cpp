//
// Created by Kurosu Chan on 2023/12/7.
//
#include "value_reading.h"

namespace protocol {

etl::expected<Command, CborError> decode_command(const uint8_t *buffer, size_t size) {
  using ue_t = etl::unexpected<CborError>;
  CborError err;
  CborParser parser;
  CborValue value;
  cbor_parser_init(buffer, size, 0, &parser, &value);
  uint8_t magic;
  err = cbor_value_get_simple_type(&value, &magic);
  if (err != CborNoError) {
    return ue_t{err};
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
    default:
      return ue_t{CborErrorUnknownType};
  }
}
}
