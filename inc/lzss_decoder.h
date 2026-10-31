#ifndef LZSS_DECODER_H__
#define LZSS_DECODER_H__

#include "lzss_parser.h"
#include <stdint.h>
#include <stdio.h>

typedef struct decoder decoder_t;

decoder_t *d_init(uint32_t window_size);
void d_destroy(decoder_t *d);

size_t d_apply_token(decoder_t *d, token_t token, uint8_t *out);
uint8_t d_get_last_byte(decoder_t *d);

#endif
