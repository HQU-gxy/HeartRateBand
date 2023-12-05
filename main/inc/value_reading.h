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
etl::expected<size_t, CborError>
encode_load_cell_reading(const float *begin,
                         const float *end,
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
}

#endif // PUNCHER_VALUE_READING_H
