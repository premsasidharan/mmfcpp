#ifndef _WAVE_HDR_H
#define _WAVE_HDR_H

#include <stdint.h>

struct Wave_params
{
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

struct Wave_data
{
    uint8_t format[4];
    uint8_t fmt_id[4];
    uint32_t size;
    Wave_params params;
    uint8_t data_id[4];
    uint32_t data_size;
};

struct Wave_hdr
{
    uint8_t riff_id[4];
    uint32_t size;
    struct Wave_data wave;
};

#endif
