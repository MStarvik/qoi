#include "qoi_fast.h"

#include <string.h>

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} qoi_px_rgb;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} qoi_px_rgba;

// typedef struct {
//   uint8_t    tag;
//   qoi_px_rgb rgb;
// } qoi_op_rgb;

// typedef struct {
//   uint8_t     tag;
//   qoi_px_rgba rgba;
// } qoi_op_rgba;

typedef struct {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  uint8_t tag   : 2;
  uint8_t index : 6;
#endif
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  uint8_t index : 6;
  uint8_t tag   : 2;
#endif
} qoi_op_index;

typedef struct {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  uint8_t tag : 2;
  uint8_t dr  : 2;
  uint8_t dg  : 2;
  uint8_t db  : 2;
#endif
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  uint8_t db  : 2;
  uint8_t dg  : 2;
  uint8_t dr  : 2;
  uint8_t tag : 2;
#endif
} qoi_op_diff;

typedef struct {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  uint8_t tag   : 2;
  uint8_t dg    : 6;
  uint8_t dr_dg : 4;
  uint8_t db_dg : 4;
#endif
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  uint8_t dg    : 6;
  uint8_t tag   : 2;
  uint8_t db_dg : 4;
  uint8_t dr_dg : 4;
#endif
} qoi_op_luma;

typedef struct {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  uint8_t tag : 2;
  uint8_t run : 6;
#endif
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  uint8_t run : 6;
  uint8_t tag : 2;
#endif
} qoi_op_run;

static inline int qoi_hash(const qoi_px_rgba* px) {
  return (px->r * 3 + px->g * 5 + px->b * 7 + px->a * 11) % 64;
}

bool qoi_decode_header(uint8_t *input, qoi_header *header) {
  memcpy(header, input, sizeof(*header));
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  header->magic = __builtin_bswap32(header->magic);
  header->width = __builtin_bswap32(header->width);
  header->height = __builtin_bswap32(header->height);
#endif
  return true;
}

bool qoi_decode(uint8_t *input, uint8_t *output, size_t size) {
  qoi_header header;
  qoi_decode_header(input, &header);
  input += sizeof(header);
  
  // const bool alpha = header.channels == 4;

  qoi_px_rgba px = {0, 0, 0, 255};
  qoi_px_rgba pxs[64] = {};

  const uint8_t *input_begin = input;
  const uint8_t *input_end = input_begin + size - 8;

  // const uint8_t *output_begin = input;
  // const uint8_t *output_end = output_begin + header.width * header.height * header.channels;
  
  while (input < input_end) {
    const int tag = *input;
    if (tag == 0b11111110) {
      *(qoi_px_rgb*)(&px) = *(qoi_px_rgb*)(++input);
      input += sizeof(qoi_px_rgb);
      pxs[qoi_hash(&px)] = px;
    } else if (tag == 0b11111111) {
      px = *(qoi_px_rgba*)(++input);
      input += sizeof(qoi_px_rgba);
      pxs[qoi_hash(&px)] = px;
    }
    else {
      const int tag = *input >> 6;
      if (tag == 0b00) {
        px = pxs[((qoi_op_index*)input)->index];
        input += sizeof(qoi_op_index);
      }
      else if (tag == 0b01) {
        px.r += ((qoi_op_diff*)input)->dr - 2;
        px.g += ((qoi_op_diff*)input)->dg - 2;
        px.b += ((qoi_op_diff*)input)->db - 2;
        input += sizeof(qoi_op_diff);
        pxs[qoi_hash(&px)] = px;
      }
      else if (tag == 0b10) {
        const int dg = ((qoi_op_luma*)input)->dg - 32;
        px.r += dg + ((qoi_op_luma*)input)->dr_dg - 8;
        px.g += dg;
        px.b += dg + ((qoi_op_luma*)input)->db_dg - 8;
        input += sizeof(qoi_op_luma);
        pxs[qoi_hash(&px)] = px;
      }
      else if (tag == 0b11) {
        for (int i = 0; i < ((qoi_op_run*)input)->run; i++) {
          if (header.channels == 4) {
            *(qoi_px_rgba*)output = px;
            output += sizeof(qoi_px_rgba);
          }
          else {
            *(qoi_px_rgb*)output = *(qoi_px_rgb*)(&px);
            output += sizeof(qoi_px_rgb);
          }
        }
        input += sizeof(qoi_op_run);
      }
    }
    if (header.channels == 4) {
      *(qoi_px_rgba*)output = px;
      output += sizeof(qoi_px_rgba);
    }
    else {
      *(qoi_px_rgb*)output = *(qoi_px_rgb*)(&px);
      output += sizeof(qoi_px_rgb);
    }
  }
  return true;
}