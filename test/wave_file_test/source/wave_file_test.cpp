#include <string.h>
#include <stdlib.h>
#include <read_wave_file.h>
#include <write_wave_file.h>

void usage();
void read_test(char* path);
void write_test(char* path);

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        usage();
        return 0;
    }
    if (0 == strcmp(argv[1], "-r"))
    {
        read_test(argv[2]);
    }
    else if (0 == strcmp(argv[1], "-w"))
    {
        write_test(argv[2]);
    }
    else
    {
        usage();
    }
    return 0;
}

void read_test(char* path)
{
    Read_wave_file file;
    if (file.open(path))
    {
        int size = file.frame_size()*1024;
        unsigned char* buffer = new unsigned char[size];
        int count = 0, size_read = 0;
        while (0 == file.is_eof())
        {
            file.read(buffer, size, size_read);
            ++count;
            printf("\n\tcount: %d, size: %d, size_read: %d", count, size, size_read);
        }
        printf("\nSample rate: %d", file.sample_rate());
        printf("\nChannel count: %d", file.channel_count());
        printf("\nBits per sample: %d", file.bits_per_sample());
        printf("\nTotal frames: %d", file.frames_count());
        file.close();
        delete [] buffer;
        buffer = 0;
    }
}

void write_test(char* path)
{
    Write_wave_file file;
    if (file.open(path, 44100, 2, 16))
    {
        unsigned char* buffer = new unsigned char[1024*file.frame_size()];
        for (int i = 0; i < 1024*file.frame_size(); i++)
        {
            buffer[i] = rand()%128;
        }
        file.write(buffer, 1024);
        file.close();
        delete [] buffer;
        buffer = 0;
    }
}

void usage()
{
    printf("\n\nUsage...\nwave_file_test -[r/w] file_path\n\n\n");
}