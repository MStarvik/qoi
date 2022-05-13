#pragma once

#include <stdint.h>

typedef struct {
  uint32_t magic;
  uint32_t width;
  uint32_t height;
  uint8_t  channels;
  uint8_t  colorspace;
} __attribute__((packed)) qoi_header;

bool qoi_encode(uint8_t* input, uint8_t* output, qoi_header* header);

bool qoi_decode_header(uint8_t* input, qoi_header* header);
bool qoi_decode(uint8_t* input, uint8_t* output, qoi_header header);