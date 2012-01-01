#ifndef _PCM_PARAMETERS_H
#define _PCM_PARAMETERS_H

struct Pcm_param
{
	unsigned int channel_count;
	unsigned int samples_per_sec;
	unsigned int avg_bytes_per_sec;
	unsigned int bits_per_sample;
};

#endif

