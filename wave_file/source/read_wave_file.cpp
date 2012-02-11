#include <string.h>
#include <read_wave_file.h>

Read_wave_file::Read_wave_file()
    :Wave_file()
{
}

Read_wave_file::~Read_wave_file()
{
    close();
}

int Read_wave_file::open(const char* path)
{
    close();
    file = fopen(path, "r");
    if (0 == file)
    {
        printf("Unable to Open file: %s\n", path);
        return 0;
    }
    fread(&hdr, 1, sizeof(Wave_hdr), file);
    int status = (memcmp(hdr.riff_id, "RIFF", 4) == 0);
    status = (status == 1)?(memcmp(hdr.wave.format, "WAVE", 4) == 0):status;
    status = (status == 1)?(memcmp(hdr.wave.fmt_id, "fmt ", 4) == 0):status;
    status = (status == 1)?(memcmp(hdr.wave.data_id, "data", 4) == 0):status;
    if (status == 0)
    {
        close();
        printf("Invalid file format: %s\n", path);
        return 0;
    }
    return 1;
}

void Read_wave_file::close()
{
    close_file();
}

int Read_wave_file::read(unsigned char* buffer, int buf_size, int& size)
{
    if (0 == file || buf_size == 0)
    {
        size = 0;
        return 0;
    }
    if (feof(file))
    {
        size = 0;
        return 0;
    }
    if (ferror(file))
    {
        size = 0;
        return 0;
    }
    size = fread(buffer, 1, buf_size, file);        
    return 1;
}
