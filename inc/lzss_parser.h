#ifndef LZSS_PARSER_H__
#define LZSS_PARSER_H__

#include <stdint.h>
#include <stdio.h>

#define P_EOF 1
#define P_OK 0

typedef struct
{
    int is_match;
    uint8_t literal;
    uint32_t len;
    uint32_t distance;
} token_t;

typedef struct parser parser_t;

parser_t *p_init(FILE *stream);
void p_destroy(parser_t *p);

int p_get_token(parser_t *p, token_t *out);
uint8_t p_get_last_byte(parser_t *p);
#endif
