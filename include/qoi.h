#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint32_t magic;
  uint32_t width;
  uint32_t height;
  uint8_t  channels;
  uint8_t  colorspace;
} __attribute__((packed)) qoi_header;

#ifdef __cplusplus
extern "C" {
#endif 

bool qoi_encode_header(qoi_header* dest, uint32_t width, uint32_t height, uint8_t channels, uint8_t colorspace);
bool qoi_encode(qoi_header header, uint8_t* dest, uint8_t* src);

bool qoi_decode_header(qoi_header* dest, uint8_t* src);
bool qoi_decode(uint8_t* dest, uint8_t* src, size_t size);

#ifdef __cplusplus
}
#endif
