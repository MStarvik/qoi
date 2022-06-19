#include "qoi.h"

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

bool qoi_encode_header(qoi_header *dest, uint32_t width, uint32_t height, uint8_t channels, uint8_t colorspace) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  dest->magic = *(uint32_t*)"qoif";
  dest->width = width;
  dest->height = height;
  dest->channels = channels;
  dest->colorspace = colorspace;
#endif
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  dest->magic = *(uint32_t*)"fioq";
  dest->width = __builtin_bswap32(width);
  dest->height = __builtin_bswap32(height);
  dest->channels = channels;
  dest->colorspace = colorspace;
#endif
  return true;
}

bool qoi_encode(qoi_header header, uint8_t *dest, uint8_t *src) {
  *(qoi_header*)dest = header;
  dest += sizeof(header);

  qoi_px_rgba px = {0, 0, 0, 255};
  qoi_px_rgba pxs[64] = {};

  const uint8_t *src_begin = src;
  const uint8_t *src_end = src_begin + header.width * header.height * header.channels;

  int run = 0;
  for (; src < src_end; src++) {
    // if () {
      
    // }
  }



}

bool qoi_decode_header(qoi_header *dest, uint8_t *src) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  *dest = *(qoi_header*)input;
#endif
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  dest->magic = __builtin_bswap32(((qoi_header*)src)->magic);
  dest->width = __builtin_bswap32(((qoi_header*)src)->width);
  dest->height = __builtin_bswap32(((qoi_header*)src)->height);
  dest->channels = ((qoi_header*)src)->channels;
  dest->colorspace = ((qoi_header*)src)->colorspace;
#endif
  return true;
}

bool qoi_decode(uint8_t *dest, uint8_t *src, size_t size) {
  qoi_header header;
  qoi_decode_header(&header, src);
  src += sizeof(header);
  
  // const bool alpha = header.channels == 4;

  qoi_px_rgba px = {0, 0, 0, 255};
  qoi_px_rgba pxs[64] = {};

  const uint8_t *src_begin = src;
  const uint8_t *src_end = src_begin + size - 8;

  // const uint8_t *dest_begin = dest;
  // const uint8_t *dest_end = dest_begin + header.width * header.height * header.channels;
  
  while (src < src_end) {
    const int tag = *src;
    if (tag == 0b11111110) {
      *(qoi_px_rgb*)(&px) = *(qoi_px_rgb*)(++src);
      src += sizeof(qoi_px_rgb);
      pxs[qoi_hash(&px)] = px;
    } else if (tag == 0b11111111) {
      px = *(qoi_px_rgba*)(++src);
      src += sizeof(qoi_px_rgba);
      pxs[qoi_hash(&px)] = px;
    }
    else {
      const int tag = *src >> 6;
      if (tag == 0b00) {
        px = pxs[((qoi_op_index*)src)->index];
        src += sizeof(qoi_op_index);
      }
      else if (tag == 0b01) {
        px.r += ((qoi_op_diff*)src)->dr - 2;
        px.g += ((qoi_op_diff*)src)->dg - 2;
        px.b += ((qoi_op_diff*)src)->db - 2;
        src += sizeof(qoi_op_diff);
        pxs[qoi_hash(&px)] = px;
      }
      else if (tag == 0b10) {
        const int dg = ((qoi_op_luma*)src)->dg - 32;
        px.r += dg + ((qoi_op_luma*)src)->dr_dg - 8;
        px.g += dg;
        px.b += dg + ((qoi_op_luma*)src)->db_dg - 8;
        src += sizeof(qoi_op_luma);
        pxs[qoi_hash(&px)] = px;
      }
      else if (tag == 0b11) {
        for (int i = 0; i < ((qoi_op_run*)src)->run; i++) {
          if (header.channels == 4) {
            *(qoi_px_rgba*)dest = px;
            dest += sizeof(qoi_px_rgba);
          }
          else {
            *(qoi_px_rgb*)dest = *(qoi_px_rgb*)(&px);
            dest += sizeof(qoi_px_rgb);
          }
        }
        src += sizeof(qoi_op_run);
      }
    }
    if (header.channels == 4) {
      *(qoi_px_rgba*)dest = px;
      dest += sizeof(qoi_px_rgba);
    }
    else {
      *(qoi_px_rgb*)dest = *(qoi_px_rgb*)(&px);
      dest += sizeof(qoi_px_rgb);
    }
  }
  return true;
}