#ifndef _WAVE_FILE_H_
#define _WAVE_FILE_H_

#include <stdio.h>
#include <wave_hdr.h>

class Wave_file
{
public:
    Wave_file();
    virtual ~Wave_file();

public:
    int data_size() const { return hdr.wave.data_size; };
    int sample_rate() const { return hdr.wave.params.sample_rate; };
    int channel_count() const { return hdr.wave.params.num_channels; };
    int bits_per_sample() const { return hdr.wave.params.bits_per_sample; };

public:
    int is_eof();
    int seek(long offset, int whence);
    virtual void close() = 0;

protected:
    void close_file();

protected:
    FILE* file;
    Wave_hdr hdr;
};

#endif

