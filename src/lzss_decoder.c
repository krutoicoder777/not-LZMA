#include "lzss_decoder.h"
#include <stdlib.h>
#include <string.h>

struct decoder
{
    uint8_t *history;
    uint32_t pos;
    uint32_t window_size;
};

decoder_t *d_init(uint32_t window_size)
{
    decoder_t *d = malloc(sizeof(decoder_t));
    if (d)
    {
        d->window_size = window_size;
        d->history = calloc(window_size, 1);
        d->pos = 0;
    }
    return d;
}

void d_destroy(decoder_t *d)
{
    if (d)
    {
        free(d->history);
        free(d);
    }
}

size_t d_apply_token(decoder_t *d, token_t token, uint8_t *out)
{
    if (!token.is_match)
    {
        out[0] = token.literal;
        d->history[d->pos % d->window_size] = token.literal;
        d->pos++;
        return 1;
    }
    else
    {
        uint32_t len = token.len;
        uint32_t dist = token.distance;

        for (uint32_t i = 0; i < len; i++)
        {
            uint8_t byte = d->history[(d->pos - dist + d->window_size) % d->window_size];
            out[i] = byte;
            d->history[d->pos % d->window_size] = byte;
            d->pos++;
        }
        return len;
    }
}

uint8_t d_get_last_byte(decoder_t *d)
{
    return d->history[(d->pos - 1 + d->window_size) % d->window_size];
}
