#include "lzss_parser.h"
#include "search_window.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_SIZE (1 << 20)
#define LOOK_AHEAD_SIZE 512

struct parser
{
    search_window_t *sw;
};

parser_t *p_init(FILE *stream)
{
    parser_t *p = malloc(sizeof(parser_t));
    if (p)
    {
        p->sw = sw_init(stream, WINDOW_SIZE, LOOK_AHEAD_SIZE);
        if (!p->sw)
        {
            free(p);
            p = NULL;
        }
    }
    return p;
}

void p_destroy(parser_t *p)
{
    sw_destroy(p->sw);
    free(p);
}

int p_get_token(parser_t *p, token_t *out)
{
    if (sw_eof(p->sw))
    {
        return P_EOF;
    }
    out->len = 0;
    out->distance = 0;
    sw_find_match(p->sw, &out->distance, &out->len);
    if (out->len >= 3)
    {
        out->is_match = 1;
        sw_shift(p->sw, out->len);
    }
    else
    {
        out->is_match = 0;
        out->literal = sw_current_byte(p->sw);
        sw_shift(p->sw, 1);
    }
    return P_OK;
}

uint8_t p_get_last_byte(parser_t *p)
{
    return sw_get_last_byte(p->sw);
}
