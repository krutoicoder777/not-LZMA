#ifndef LZMA_H__
#define LZMA_H__

#include <stdint.h>
#include <stdio.h>

#define LZMA_OK 0
#define LZMA_ERR_MEM 1
#define LZMA_ERR_IO 2

int lzma_encode_stream(FILE *in, FILE *out);
int lzma_decode_stream(FILE *in, FILE *out, uint64_t original_size);

#endif
