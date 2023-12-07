//
// Created by Kurosu Chan on 2023/12/5.
//

#ifndef PUNCHER_VALUE_READING_H
#define PUNCHER_VALUE_READING_H

#include <etl/expected.h>
#include <cbor.h>

namespace protocol {
// https://gist.github.com/soburi/4e1cc77df363e52ff0f366aeb23dac39
// https://www.rfc-editor.org/rfc/rfc8949.html#name-cbor-data-models
// https://www.rfc-editor.org/rfc/rfc8949.html#name-cbor-tags-registry
// https://www.rfc-editor.org/rfc/rfc8949.html#name-tagging-of-items
constexpr uint8_t LOAD_CELL_READING_MAGIC = 0x10;

enum class Command {
  ONCE        = 0x12,
  SUCCESSIVE  = 0x13,
  STOP        = 0x14,
  TARE        = 0x20,
  BTN_DISABLE = 0x30,
  BTN_ENABLE  = 0x31,
  UNKNOWN,
};

/**
 * @brief encode a load cell reading into a CBOR byte array
 * @tparam It an iterator type that points to a floating point value
 */
template <typename It>
  requires std::is_floating_point_v<typename std::iterator_traits<It>::value_type>
etl::expected<size_t, CborError>
encode_load_cell_reading(const It begin,
                         const It end,
                         uint8_t *buffer,
                         size_t size) {
  using ue_t = etl::unexpected<CborError>;
  CborError err;
  CborEncoder encoder;
  cbor_encoder_init(&encoder, buffer, size, 0);
  auto len = std::distance(begin, end);
  err      = cbor_encode_simple_value(&encoder, LOAD_CELL_READING_MAGIC);
  if (err != CborNoError) {
    return ue_t{err};
  }
  CborEncoder container;
  err = cbor_encoder_create_array(&encoder, &container, len);
  if (err != CborNoError) {
    return ue_t{err};
  }
  for (auto it = begin; it != end; ++it) {
    err = cbor_encode_float_as_half_float(&container, *it);
    if (err != CborNoError) {
      return ue_t{err};
    }
  }
  err = cbor_encoder_close_container_checked(&encoder, &container);
  if (err != CborNoError) {
    return ue_t{err};
  }
  size_t sz = cbor_encoder_get_buffer_size(&encoder, buffer);
  return sz;
};

etl::expected<Command, CborError>
decode_command(const uint8_t *buffer, size_t size);

}

#endif // PUNCHER_VALUE_READING_H
