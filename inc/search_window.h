#ifndef SEARCH_WINDOW_H__
#define SEARCH_WINDOW_H__

#include <stdint.h>
#include <stdio.h>

typedef struct search_window search_window_t;

search_window_t *sw_init(FILE *stream, uint32_t window_size, uint32_t look_ahead_size);
void sw_destroy(search_window_t *sw);

void sw_find_match(search_window_t *sw, uint32_t *distance, uint32_t *len);
uint8_t sw_current_byte(search_window_t *sw);
void sw_shift(search_window_t *sw, uint32_t len);
int sw_eof(search_window_t *sw);
uint8_t sw_get_last_byte(search_window_t *sw);

#endif
