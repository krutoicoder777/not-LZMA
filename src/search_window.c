#include "search_window.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct search_window
{
    uint8_t *buf;
    uint32_t buf_size;
    uint32_t window_size;
    uint32_t look_ahead_size;
    uint32_t hash_size;
    uint32_t pos;
    uint32_t head;
    uint32_t tail;
    int is_eof;
    FILE *stream;
    int32_t *head_table;
    int32_t *prev;
    uint8_t last_byte;
};

search_window_t *sw_init(FILE *stream, uint32_t window_size, uint32_t look_ahead_size)
{
    search_window_t *sw = malloc(sizeof(search_window_t));
    if (sw)
    {
        sw->pos = 0;
        sw->head = 0;
        sw->tail = 0;
        sw->is_eof = 0;
        sw->window_size = window_size;
        sw->look_ahead_size = look_ahead_size;
        sw->hash_size = window_size;
        sw->buf_size = window_size + look_ahead_size;

        if (window_size < 1 || look_ahead_size < 1)
        {
            free(sw);
            return NULL;
        }

        sw->stream = stream;
        sw->buf = calloc(sw->buf_size, sizeof(uint8_t));
        if (!sw->buf)
        {
            free(sw);
            return NULL;
        }

        sw->prev = malloc(sw->buf_size * sizeof(int32_t));
        if (!sw->prev)
        {
            free(sw->buf);
            free(sw);
            return NULL;
        }

        sw->head_table = malloc(sw->hash_size * sizeof(int32_t));
        if (!sw->head_table)
        {
            free(sw->prev);
            free(sw->buf);
            free(sw);
            return NULL;
        }

        for (uint32_t i = 0; i < sw->hash_size; i++)
        {
            sw->head_table[i] = -1;
        }
        for (uint32_t i = 0; i < sw->buf_size; i++)
        {
            sw->prev[i] = -1;
        }

        uint32_t read_bytes = (uint32_t)fread(sw->buf, 1, sw->buf_size, stream);
        sw->head = read_bytes;

        sw->last_byte = 0;
    }
    return sw;
}

void sw_destroy(search_window_t *sw)
{
    free(sw->buf);
    free(sw->head_table);
    free(sw->prev);
    free(sw);
}

static uint32_t djb2(uint8_t c1, uint8_t c2, uint8_t c3)
{
    uint32_t h = 5381;
    h = ((h << 5) + h) + c1;
    h = ((h << 5) + h) + c2;
    h = ((h << 5) + h) + c3;
    return h;
}

static void sw_insert(search_window_t *sw)
{
    if (sw->pos + 2 >= sw->head)
    {
        return;
    }

    uint32_t p0 = sw->pos % sw->buf_size;
    uint32_t p1 = (sw->pos + 1) % sw->buf_size;
    uint32_t p2 = (sw->pos + 2) % sw->buf_size;

    uint32_t h = djb2(sw->buf[p0], sw->buf[p1], sw->buf[p2]) % sw->hash_size;

    sw->prev[sw->pos % sw->buf_size] = sw->head_table[h];
    sw->head_table[h] = (int32_t)sw->pos;
}

void sw_find_match(search_window_t *sw, uint32_t *distance, uint32_t *len)
{
    *distance = 0;
    *len = 0;

    if (sw->pos + 2 >= sw->head)
    {
        return;
    }

    uint32_t p0 = sw->pos % sw->buf_size;
    uint32_t p1 = (sw->pos + 1) % sw->buf_size;
    uint32_t p2 = (sw->pos + 2) % sw->buf_size;

    uint32_t h = djb2(sw->buf[p0], sw->buf[p1], sw->buf[p2]) % sw->hash_size;
    int32_t candidate = sw->head_table[h];
    uint32_t best_len = 0;
    uint32_t best_dist = 0;

    for (uint32_t i = 0; i < 32; i++)
    {
        if (candidate == -1)
        {
            break;
        }

        if (sw->pos - (uint32_t)candidate >= sw->window_size)
        {
            break;
        }

        uint32_t match_len = 0;
        while (sw->pos + match_len < sw->head && match_len < 258 &&
               sw->buf[((uint32_t)candidate + match_len) % sw->buf_size] ==
                   sw->buf[(sw->pos + match_len) % sw->buf_size])
        {
            match_len++;
        }

        if (match_len > best_len && match_len >= 3)
        {
            best_len = match_len;
            best_dist = sw->pos - (uint32_t)candidate;
        }

        if (best_len == 258)
        {
            break;
        }

        candidate = sw->prev[(uint32_t)candidate % sw->buf_size];
    }

    *distance = best_dist;
    *len = best_len;
}

uint8_t sw_current_byte(search_window_t *sw)
{
    return sw->buf[sw->pos % sw->buf_size];
}

void sw_shift(search_window_t *sw, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        sw->last_byte = sw_current_byte(sw);
        sw_insert(sw);
        sw->pos++;
    }

    while (sw->pos + sw->look_ahead_size > sw->head && !sw->is_eof)
    {
        int c = fgetc(sw->stream);
        if (c == EOF)
        {
            sw->is_eof = 1;
            break;
        }

        sw->buf[sw->head % sw->buf_size] = (uint8_t)c;
        sw->head++;

        if (sw->head - sw->tail > sw->buf_size)
        {
            sw->tail++;
        }
    }
}

int sw_eof(search_window_t *sw)
{
    if (sw->pos >= sw->head)
    {
        return 1;
    }
    return 0;
}

uint8_t sw_get_last_byte(search_window_t *sw)
{
    return sw->last_byte;
}
