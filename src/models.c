#include "models.h"

void model_init(model_t *m)
{
    for (int i = 0; i < 256; i++)
    {
        m->freq[i] = 1;
    }
    m->total = 256;
}

uint32_t model_cum(model_t *m, int symbol)
{
    uint32_t cum = 0;
    for (int i = 0; i < symbol; i++)
    {
        cum += m->freq[i];
    }
    return cum;
}

uint32_t model_freq(model_t *m, int symbol)
{
    return m->freq[symbol];
}

int model_sym(model_t *m, uint32_t target)
{
    uint32_t cum = 0;
    for (int i = 0; i < 256; i++)
    {
        cum += m->freq[i];
        if (target < cum)
        {
            return i;
        }
    }
    return 255;
}

void model_update(model_t *m, int symbol)
{
    m->freq[symbol]++;
    m->total++;
    if (m->total >= MODEL_TOTAL_MAX)
    {
        m->total = 0;
        for (int i = 0; i < 256; i++)
        {
            m->freq[i] = (m->freq[i] + 1) / 2;
            m->total += m->freq[i];
        }
    }
}

void lit_model_init(lit_model_t *m)
{
    for (int i = 0; i < 256; i++)
    {
        model_init(&m->ctx[i]);
    }
}

uint32_t lit_model_cum(lit_model_t *m, uint8_t prev_byte, int symbol)
{
    return model_cum(&m->ctx[prev_byte], symbol);
}

uint32_t lit_model_freq(lit_model_t *m, uint8_t prev_byte, int symbol)
{
    return model_freq(&m->ctx[prev_byte], symbol);
}

int lit_model_sym(lit_model_t *m, uint8_t prev_byte, uint32_t target)
{
    return model_sym(&m->ctx[prev_byte], target);
}

void lit_model_update(lit_model_t *m, uint8_t prev_byte, int symbol)
{
    model_update(&m->ctx[prev_byte], symbol);
}

void slot_model_init(slot_model_t *m)
{
    for (int i = 0; i < 64; i++)
    {
        m->freq[i] = 1;
    }
    m->total = 64;
}

uint32_t slot_model_cum(slot_model_t *m, int symbol)
{
    uint32_t cum = 0;
    for (int i = 0; i < symbol; i++)
    {
        cum += m->freq[i];
    }
    return cum;
}

uint32_t slot_model_freq(slot_model_t *m, int symbol)
{
    return m->freq[symbol];
}

int slot_model_sym(slot_model_t *m, uint32_t target)
{
    uint32_t cum = 0;
    for (int i = 0; i < 64; i++)
    {
        cum += m->freq[i];
        if (target < cum)
        {
            return i;
        }
    }
    return 63;
}

void slot_model_update(slot_model_t *m, int symbol)
{
    m->freq[symbol]++;
    m->total++;
    if (m->total >= 65536)
    {
        m->total = 0;
        for (int i = 0; i < 64; i++)
        {
            m->freq[i] = (m->freq[i] + 1) / 2;
            m->total += m->freq[i];
        }
    }
}

static const uint32_t slot_base[64] = {
    1,          2,          3,          4,         5,         7,         9,         13,        17,        25,
    33,         49,         65,         97,        129,       193,       257,       385,       513,       769,
    1025,       1537,       2049,       3073,      4097,      6145,      8193,      12289,     16385,     24577,
    32769,      49153,      65537,      98305,     131073,    196609,    262145,    393217,    524289,    786433,
    1048577,    1572865,    2097153,    3145729,   4194305,   6291457,   8388609,   12582913,  16777217,  25165825,
    33554433,   50331649,   67108865,   100663297, 134217729, 201326593, 268435457, 402653185, 536870913, 805306369,
    1073741825, 1610612737, 2147483649, 3221225473};

int slot(uint32_t dist)
{
    if (dist < 5)
    {
        return dist - 1;
    }
    int slot = 4;
    while (dist >= slot_base[slot + 1])
    {
        slot++;
    }
    return slot;
}

uint32_t base_dist(int slot)
{
    return slot_base[slot];
}

int extra_bits(int slot) {
    if (slot < 4) return 0;
    uint32_t range = slot_base[slot + 1] - slot_base[slot];
    int bits = 0;
    while ((1u << bits) < range) bits++;
    return bits;
}
