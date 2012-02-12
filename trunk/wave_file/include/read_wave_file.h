#ifndef _READ_WAVE_FILE_H_
#define _READ_WAVE_FILE_H_

#include <wave_file.h>

class Read_wave_file:public Wave_file
{
public:
    Read_wave_file();
    ~Read_wave_file();

public:
    int open(const char* path);
    int read(unsigned char* buffer, int buff_size, int& frames_read);
    int seek(long offset, int whence);
    void close();

};

#endif

