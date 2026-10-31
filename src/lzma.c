#include "lzma.h"
#include "lzss_decoder.h"
#include "lzss_parser.h"
#include "models.h"
#include "range_coder.h"
#include "range_decoder.h"
#include <stdint.h>
#include <stdio.h>

int lzma_encode_stream(FILE *in, FILE *out)
{
    parser_t *p = p_init(in);
    if (!p)
    {
        return LZMA_ERR_MEM;
    }

    range_coder_t *rc = rc_init(out);
    if (!rc)
    {
        p_destroy(p);
        return LZMA_ERR_MEM;
    }

    model_t len_model, dist_high_model, dist_mid_model, dist_low_model;
    lit_model_t lit_model;

    model_init(&len_model);
    model_init(&dist_high_model);
    model_init(&dist_mid_model);
    model_init(&dist_low_model);
    lit_model_init(&lit_model);

    uint8_t prev_byte = 0;
    token_t token;

    while (p_get_token(p, &token) == P_OK)
    {
        if (token.is_match)
        {
            rc_update(rc, 1, 1, 2);

            uint32_t len_sym = token.len - 3;
            rc_update(rc, model_cum(&len_model, len_sym), model_freq(&len_model, len_sym), len_model.total);
            model_update(&len_model, len_sym);

            uint32_t d_high = (token.distance >> 16) & 0xFF;
            uint32_t d_mid = (token.distance >> 8) & 0xFF;
            uint32_t d_low = token.distance & 0xFF;

            rc_update(rc, model_cum(&dist_high_model, d_high), model_freq(&dist_high_model, d_high),
                      dist_high_model.total);
            model_update(&dist_high_model, d_high);

            rc_update(rc, model_cum(&dist_mid_model, d_mid), model_freq(&dist_mid_model, d_mid), dist_mid_model.total);
            model_update(&dist_mid_model, d_mid);

            rc_update(rc, model_cum(&dist_low_model, d_low), model_freq(&dist_low_model, d_low), dist_low_model.total);
            model_update(&dist_low_model, d_low);

            prev_byte = p_get_last_byte(p);
        }
        else
        {
            rc_update(rc, 0, 1, 2);

            uint32_t lit = token.literal;
            rc_update(rc, lit_model_cum(&lit_model, prev_byte, lit), lit_model_freq(&lit_model, prev_byte, lit),
                      lit_model.ctx[prev_byte].total);
            lit_model_update(&lit_model, prev_byte, lit);

            prev_byte = (uint8_t)lit;
        }
    }

    rc_flush(rc);
    rc_destroy(rc);
    p_destroy(p);
    return LZMA_OK;
}

int lzma_decode_stream(FILE *in, FILE *out, uint64_t original_size)
{
    range_decoder_t *rd = rd_init(in);
    if (!rd)
    {
        return LZMA_ERR_MEM;
    }

    decoder_t *d = d_init(1 << 20);
    if (!d)
    {
        rd_destroy(rd);
        return LZMA_ERR_MEM;
    }

    model_t len_model, dist_high_model, dist_mid_model, dist_low_model;
    lit_model_t lit_model;

    model_init(&len_model);
    model_init(&dist_high_model);
    model_init(&dist_mid_model);
    model_init(&dist_low_model);
    lit_model_init(&lit_model);

    uint8_t prev_byte = 0;
    uint64_t decoded_bytes = 0;
    uint8_t out_buf[258];

    while (decoded_bytes < original_size)
    {
        uint32_t target = rd_target(rd, 2);
        int is_match = (int)target;
        rd_update(rd, is_match, 1);

        if (is_match)
        {
            target = rd_target(rd, len_model.total);
            int len_sym = model_sym(&len_model, target);
            rd_update(rd, model_cum(&len_model, len_sym), model_freq(&len_model, len_sym));
            model_update(&len_model, len_sym);
            uint32_t len = len_sym + 3;

            target = rd_target(rd, dist_high_model.total);
            int d_high = model_sym(&dist_high_model, target);
            rd_update(rd, model_cum(&dist_high_model, d_high), model_freq(&dist_high_model, d_high));
            model_update(&dist_high_model, d_high);

            target = rd_target(rd, dist_mid_model.total);
            int d_mid = model_sym(&dist_mid_model, target);
            rd_update(rd, model_cum(&dist_mid_model, d_mid), model_freq(&dist_mid_model, d_mid));
            model_update(&dist_mid_model, d_mid);

            target = rd_target(rd, dist_low_model.total);
            int d_low = model_sym(&dist_low_model, target);
            rd_update(rd, model_cum(&dist_low_model, d_low), model_freq(&dist_low_model, d_low));
            model_update(&dist_low_model, d_low);

            uint32_t distance = (d_high << 16) | (d_mid << 8) | d_low;

            token_t token;
            token.is_match = 1;
            token.len = len;
            token.distance = distance;

            size_t written = d_apply_token(d, token, out_buf);
            fwrite(out_buf, 1, written, out);

            prev_byte = d_get_last_byte(d);
            decoded_bytes += len;
        }
        else
        {
            target = rd_target(rd, lit_model.ctx[prev_byte].total);
            int lit = lit_model_sym(&lit_model, prev_byte, target);
            rd_update(rd, lit_model_cum(&lit_model, prev_byte, lit), lit_model_freq(&lit_model, prev_byte, lit));
            lit_model_update(&lit_model, prev_byte, lit);

            token_t token;
            token.is_match = 0;
            token.literal = (uint8_t)lit;

            size_t written = d_apply_token(d, token, out_buf);
            fwrite(out_buf, 1, written, out);

            prev_byte = (uint8_t)lit;
            decoded_bytes += 1;
        }
    }

    d_destroy(d);
    rd_destroy(rd);
    return LZMA_OK;
}
