#include "qoi.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>

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

typedef union {
  qoi_px_rgb  rgb;
  qoi_px_rgba rgba;
} qoi_px;

typedef struct {
  uint8_t    tag;
  qoi_px_rgb rgb;
} qoi_op_rgb;

typedef struct {
  uint8_t     tag;
  qoi_px_rgba rgba;
} qoi_op_rgba;

typedef struct {
  unsigned int index : 6;
  unsigned int tag   : 2;
} qoi_op_index;

typedef struct {
  unsigned int dr  : 2;
  unsigned int dg  : 2;
  unsigned int db  : 2;
  unsigned int tag : 2;
} qoi_op_diff;

typedef struct {
  unsigned int dg    : 6;
  unsigned int dr_dg : 4;
  unsigned int db_dg : 4;
  unsigned int tag   : 2;
} qoi_op_luma;

typedef struct {
  unsigned int run : 6;
  unsigned int tag : 2;
} qoi_op_run;

typedef union {
  // uint8_t     tag;
  qoi_op_rgb  rgb;
  qoi_op_rgba rgba;
} qoi_op_long;

typedef union {
  // uint8_t      tag : 2;
  qoi_op_index index;
  qoi_op_diff  diff;
  qoi_op_luma  luma;
  qoi_op_run   run;
} qoi_op_short;

void qoi_encode(uint8_t* input, uint8_t* output, qoi_header header) {
  header.width = __builtin_bswap32(header.width);
  header.height = __builtin_bswap32(header.height);
  memcpy(output, &header, sizeof(qoi_header));
  output += sizeof(qoi_header);

  uint8_t index[64];

  const uint8_t* end = input + header.width * header.height;
  for (; input < end; input++) {
    
  }
}

bool qoi_decode_header(uint8_t *input, qoi_header *header) {
  *header = *reinterpret_cast<qoi_header*>(input);
  header->magic = __builtin_bswap32(header->magic);
  header->width = __builtin_bswap32(header->width);
  header->height = __builtin_bswap32(header->height);
  return true;
}

static inline int qoi_hash(const qoi_px_rgba &px) {
  return (px.r * 3 + px.g * 5 + px.b * 7 + px.a * 11) % 64;
}

std::ostream &operator<<(std::ostream &ost, const qoi_px_rgb &px) {
  return ost << "r: " << static_cast<int>(px.r)
             << " g: " << static_cast<int>(px.g)
             << " b: " << static_cast<int>(px.b);
}

std::ostream &operator<<(std::ostream &ost, const qoi_px_rgba &px) {
  return ost << "r: " << static_cast<int>(px.r)
             << " g: " << static_cast<int>(px.g)
             << " b: " << static_cast<int>(px.b)
             << " a: " << static_cast<int>(px.a);
}

#if 0
void qoi_decode(uint8_t* input, uint8_t* output, qoi_header header) {
  const uint8_t* begin = output;
  const uint8_t* end = begin + header.width * header.height * header.channels - 8;

  qoi_px px = {.rgba={0, 0, 0, 255}};
  qoi_px pxs[64] = {0};

  input += sizeof(header);

  int stop_bytes = 0;
  while (output < end) {
    // const qoi_op_long* op = reinterpret_cast<qoi_op_long*>(input);
    // if (stop_bytes >= 7) {
    //   if (op->tag == 0b00000001) {
    //     break;
    //   }
    // }

    // if (op->tag == 0b00000000) {
    //   stop_bytes++;
    //   input++;
    //   continue;
    // }
    // else {
    //   stop_bytes = 0;
    // }

    if (*input == 0b11111110) {
      const qoi_op_rgb* op = reinterpret_cast<qoi_op_rgb*>(input);
      px.rgb = op->rgb;
      std::cout << "RGB " << px.rgb << "\n";
      pxs[qoi_hash(px.rgba)] = px;
      reinterpret_cast<qoi_px*>(output)->rgba = px.rgba;
      input += sizeof(op->rgb);
      output += sizeof(px);
    }
    else if (*input == 0b11111111) {
      const qoi_op_rgba* op = reinterpret_cast<qoi_op_rgba*>(input);
      std::cout << "RGBA " << px.rgba << "\n";
      px.rgba = op->rgba;
      pxs[qoi_hash(px.rgba)] = px;
      reinterpret_cast<qoi_px*>(output)->rgba = px.rgba;
      input += sizeof(op->rgba);
      output += sizeof(px);
    }
    else {
      const qoi_op_short* op = reinterpret_cast<qoi_op_short*>(input);
      const int tag = *input >> 6;
      if (tag == 0b00) {
        std::cout << "INDEX " << static_cast<int>(op->index.index) << "\n";
        px = pxs[op->index.index];
        reinterpret_cast<qoi_px*>(output)->rgba = px.rgba;
        input += sizeof(op->index);
        output += sizeof(px);
      }
      else if (tag == 0b01) {
        std::cout << "DIFF " 
                  << "dr: " << static_cast<int>(op->diff.dr) - 2
                  << " dg: " << static_cast<int>(op->diff.dg) - 2
                  << " db: " << static_cast<int>(op->diff.db) - 2 << "\n";
        px.rgb.r += op->diff.dr - 2;
        px.rgb.g += op->diff.dg - 2;
        px.rgb.b += op->diff.db - 2;
        pxs[qoi_hash(px.rgba)] = px;
        reinterpret_cast<qoi_px*>(output)->rgba = px.rgba;
        input += sizeof(op->diff);
        output += sizeof(px);
      }
      else if (tag == 0b10) {
        std::cout << "LUMA "
                  << "dg: " << static_cast<int>(op->luma.dg) - 32
                  << " dr_dg: " << static_cast<int>(op->luma.dr_dg) - 8
                  << " db_dg: " << static_cast<int>(op->luma.db_dg) - 8 << "\n";
        const int dg = static_cast<int>(op->luma.dg) - 32;
        px.rgb.r += dg - 8 + static_cast<int>(op->luma.dr_dg);
        px.rgb.g += dg;
        px.rgb.b += dg - 8 + static_cast<int>(op->luma.db_dg);
        pxs[qoi_hash(px.rgba)] = px;
        reinterpret_cast<qoi_px*>(output)->rgba = px.rgba;
        input += sizeof(op->luma);
        output += sizeof(px);
      }
      else if (tag == 0b11) {
        std::cout << "RUN " << static_cast<int>(op->run.run) + 1 << "\n";
        input += sizeof(op->run);
        const int run = op->run.run;
        for (int i = -1; i < run; i++) {
          reinterpret_cast<qoi_px*>(output)->rgba = px.rgba;
          output += sizeof(px);
        }
      }
    }
  }
}
#endif

bool qoi_decode(uint8_t* input, uint8_t* output, qoi_header header) {
  const uint8_t* begin = output;
  const uint8_t* end = begin + header.width * header.height * 4;

  qoi_px px = {.rgba={0, 0, 0, 255}};
  qoi_px pxs[64] = {0};

  input += sizeof(header);

  int stop_bytes = 0;
  while (output < end) {
    // const qoi_op_long* op = reinterpret_cast<qoi_op_long*>(input);
    // if (stop_bytes >= 7) {
    //   if (op->tag == 0b00000001) {
    //     break;
    //   }
    // }

    // if (op->tag == 0b00000000) {
    //   stop_bytes++;
    //   input++;
    //   continue;
    // }
    // else {
    //   stop_bytes = 0;
    // }

    if (*input == 0b11111110) {
      px.rgb.r = *(++input);
      px.rgb.g = *(++input);
      px.rgb.b = *(++input);
      input++;
      pxs[qoi_hash(px.rgba)] = px;
      *reinterpret_cast<qoi_px*>(output) = px;
      output += sizeof(px);
    }
    else if (*input == 0b11111111) {
      px.rgba.r = *(++input);
      px.rgba.g = *(++input);
      px.rgba.b = *(++input);
      px.rgba.a = *(++input);
      input++;
      pxs[qoi_hash(px.rgba)] = px;
      *reinterpret_cast<qoi_px*>(output) = px;
      output += sizeof(px);
    }
    else {
      // const qoi_op_short* op = reinterpret_cast<qoi_op_short*>(input);
      const int tag = *input >> 6;
      if (tag == 0b00) { // INDEX
        px = pxs[*input & 0b00111111];
        input++;
        *reinterpret_cast<qoi_px*>(output) = px;
        output += sizeof(px);
      }
      else if (tag == 0b01) { // DIFF
        px.rgb.r += static_cast<int>((*input & 0b00110000) >> 4) - 2;
        px.rgb.g += static_cast<int>((*input & 0b00001100) >> 2) - 2;
        px.rgb.b += static_cast<int>(*input & 0b00000011) - 2;
        input++;
        pxs[qoi_hash(px.rgba)] = px;
        *reinterpret_cast<qoi_px*>(output) = px;
        output += sizeof(px);
      }
      else if (tag == 0b10) { // LUMA
        const int dg = static_cast<int>(*input & 0b00111111) - 32;
        input++;
        px.rgb.r += dg + static_cast<int>(*input >> 4) - 8;
        px.rgb.g += dg;
        px.rgb.b += dg + static_cast<int>(*input & 0b00001111) - 8;
        input++;
        pxs[qoi_hash(px.rgba)] = px;
        *reinterpret_cast<qoi_px*>(output) = px;
        output += sizeof(px);
      }
      else if (tag == 0b11) { // RUN
        const int run = (*input & 0b00111111) + 1;
        input++;
        for (int i = 0; i < run; i++) {
          *reinterpret_cast<qoi_px*>(output) = px;
          output += sizeof(px);
        }
      }
    }
  }
  return true;
}