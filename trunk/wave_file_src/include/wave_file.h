#ifndef _WAVE_FILE_H
#define _WAVE_FILE_H

class Wave_file
{
public:
	Wave_file(const char* path = 0);
	~Wave_file();
	
public:
	unsigned int channels() const { return channel_count; };
	unsigned int bits_per_sample() const { return _bits_per_sample; };
	unsigned int samples_per_second() const { return samples_per_sec; };
	unsigned int average_bytes_per_second() const { return avg_bytes_per_sec; };

	int open(const char* path);
	void close();

	unsigned long read(char* buffer, unsigned long size);
	
protected:
	char* find_chunk(const char* buffer, const char* chunk_data, int size);
	
protected:
	int file;
	char* file_path;
	unsigned int audio_bytes;
	unsigned long long data_start;
	
	unsigned int dw_size;
	unsigned short int format_tag;
	unsigned short int channel_count;
	unsigned int samples_per_sec;
	unsigned int avg_bytes_per_sec;
	unsigned short int block_align;
	unsigned short int _bits_per_sample;
};

#endif

