#ifndef _WRITE_WAVE_FILE_H_
#define _WRITE_WAVE_FILE_H_

#include <wave_file.h>

class Write_wave_file:public Wave_file
{
public:
    Write_wave_file();
    ~Write_wave_file();

public:
    int open(char* path, int sample_rate, int channels, int bits_per_sample);
    int write(unsigned char* buffer, int size);
    void close();

};

#endif

