#include <string.h>
#include <wave_file.h>

Wave_file::Wave_file()
    :file(0)
{
    memset(&hdr, 0, sizeof(Wave_hdr));
}

Wave_file::~Wave_file()
{
}

void Wave_file::close_file()
{
    if (0 != file)
    {
        fclose(file); file = 0;
        memset(&hdr, 0, sizeof(Wave_hdr));
    }
}

int Wave_file::is_eof()
{
    if (0 == file)
    {
        return 0;
    }
    return (0 != feof(file));
}

int Wave_file::seek(long offset, int whence)
{
    if (0 == file)
    {
        return 0;
    }
    fseek(file, offset, whence);
    return 1;
}
