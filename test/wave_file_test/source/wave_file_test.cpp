#include <string.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#include <read_wave_file.h>
#include <write_wave_file.h>

void usage();
void play_test(char* path);
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
    else if (0 == strcmp(argv[1], "-p"))
    {
        play_test(argv[2]);
    }
    else
    {
        usage();
    }
    return 0;
}

snd_pcm_format_t format(unsigned int bps)
{
    snd_pcm_format_t fmt = SND_PCM_FORMAT_U8;

    switch (bps)
    {
        case 8:
            fmt = SND_PCM_FORMAT_U8;
            break;
        case 16:
            fmt = SND_PCM_FORMAT_S16_LE;
            break;
        case 24:
            fmt = SND_PCM_FORMAT_S24_LE;
            break;
        case 32:
            fmt = SND_PCM_FORMAT_S32_LE;
            break;
    }
    return fmt;
}

void usage()
{
    printf("\n\nUsage...\nwave_file_test -[r/w] file_path\n\n\n");
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

void play_test(char* path)
{
    Read_wave_file file;
    if (file.open(path))
    {
		struct timespec time;
    	snd_pcm_t* pcm_handle = 0;
		unsigned long available = 0;
		unsigned long buffer_size = 0, period_size = 0;
		int error = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
		error = (error < 0)?error:snd_pcm_set_params(pcm_handle, 
									format(file.bits_per_sample()), 
									SND_PCM_ACCESS_RW_INTERLEAVED, 
									file.channel_count(), 
									file.sample_rate(), 0, 0);
		error = (error <0)?error:snd_pcm_get_params(pcm_handle, 
							&buffer_size, &period_size);
		error = (error < 0)?error:snd_pcm_prepare(pcm_handle);
		error = (error < 0)?error:snd_pcm_reset(pcm_handle);
		printf("\n\tBuffer Size: %ld, Period Size: %ld", buffer_size, period_size);
		if (error >= 0)
		{
        	int size = file.frame_size()*1024;
        	unsigned char* buffer = new unsigned char[size];
        	int count = 0, size_read = 0;
        	while (0 == file.is_eof())
        	{
            	++count;
            	file.read(buffer, size, size_read);
				snd_pcm_htimestamp(pcm_handle, &available, &time);
            	printf("\n\tcount: %d, size: %d, size_read: %d, avail. frames: %ld, time: %d:%d", count, size, size_read, available, time.tv_sec, time.tv_nsec);
				error = snd_pcm_writei(pcm_handle, buffer, 1024);
				snd_pcm_htimestamp(pcm_handle, &available, &time);
				error = (error >= 0)?error:snd_pcm_recover(pcm_handle, error, 0);
            	printf("\n\tcount: %d, size: %d, size_read: %d, avail. frames: %ld, time: %d:%d", count, size, size_read, available, time.tv_sec, time.tv_nsec);
        	}
        	delete [] buffer;
        	buffer = 0;
		}
		snd_pcm_close(pcm_handle);
		pcm_handle = 0;
        printf("\nSample rate: %d", file.sample_rate());
        printf("\nChannel count: %d", file.channel_count());
        printf("\nBits per sample: %d", file.bits_per_sample());
        printf("\nTotal frames: %d", file.frames_count());
        file.close();
    }
}

