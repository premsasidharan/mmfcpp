/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <wave_file.h>

#define BUFFER_SIZE 1024
#define PCM_WAVE_FORMAT 1
#define WAVE_HEADER_SIZE (sizeof(unsigned int)+sizeof(unsigned short int)+sizeof(unsigned short int)\
	+sizeof(unsigned int)+sizeof(unsigned int)+sizeof(unsigned short int)+sizeof(unsigned short int))

Wave_file::Wave_file(const char *path)
	:file(-1)
	, file_path(0)
	, audio_bytes(0)
	, data_start(0)
	, dw_size(0)
	, format_tag(0)
	, channel_count(0)
	, samples_per_sec(0)
	, avg_bytes_per_sec(0)
	, block_align(0)
	, _bits_per_sample(0)
{
	if (0 != path)
	{
		open(path);
	}
}

Wave_file::~Wave_file()
{
	close();
}

int Wave_file::open(const char *path)
{
	char *data;
	unsigned char ubuff[4];
	char buffer[BUFFER_SIZE];
	unsigned long data_bytes;

	close();

	file = ::open(path, O_RDONLY);
	if (file == -1)
	{
		printf("\nOpen Failed");
		goto open_error;
	}
	
	if (::read(file, buffer, BUFFER_SIZE) != BUFFER_SIZE)
	{
		printf("\nRead Failed");
		goto open_error;
	}

	if (find_chunk(buffer, "RIFF", BUFFER_SIZE) != buffer) 
	{
		printf("\nChunk RIFF not found");
		goto open_error;
	}

	if (find_chunk(buffer, "WAVE", BUFFER_SIZE) == 0) 
	{
		printf("\nChunk WAVE not found");
		goto open_error;
	}

	data = find_chunk(buffer, "fmt ", BUFFER_SIZE) ;
	if (0 == data) 
	{
		printf("\nChunk fmt not found");
		goto open_error;
	}

	data += 4 ;	// Move past "fmt ".
	memcpy(&dw_size, data, WAVE_HEADER_SIZE);
	
//	printf("\n\tm_dwSize :%u", dw_size);
	printf("\n\tm_wFormatTag :%u", format_tag);
	printf("\n\tchannels :%u", channel_count);
	printf("\n\tSamplesPerSec :%u", samples_per_sec);
	printf("\n\tAvgBytesPerSec :%u", avg_bytes_per_sec);
//	printf("\n\tm_wBlockAlign :%u", block_align);
	printf("\n\tBitsPerSample :%u", _bits_per_sample);
	
#if 0
	if (dw_size != (WAVE_HEADER_SIZE-sizeof(unsigned int))) 
	{
		printf("\nFormat Error");
		goto open_error;
	}

	if (format_tag != PCM_WAVE_FORMAT) 
	{
		printf("\nNot PCM");
		goto open_error;
	}
#endif

	data = find_chunk(data, "data", 1024) ;
	if (0 == data) 
	{
		printf("\nChunk data not found");
		goto open_error;
	}

	data += 4 ;	/* Move past "data".*/
	memcpy(&data_bytes, data, sizeof(unsigned long)) ;

	data_start = ((unsigned long)(data + 4)) - ((unsigned long)(&(buffer[0]))) ;

	printf("\n\n\tdata_start :%lld", data_start);
	data_start -= 4;
// 	if (samples_per_sec != avg_bytes_per_sec/block_align) 
// 	{
// 		return;  //WR_BADFORMATDATA ;
// 	}
// 
// 	if (samples_per_sec != avg_bytes_per_sec/channel_count / ((_bits_per_sample == 16) ? 2 : 1)) 
// 	{
// 		return;//  WR_BADFORMATDATA ;
// 	}
	lseek(file, data_start, SEEK_SET);
	::read(file, ubuff, 4);
	
	audio_bytes = ubuff[3];
	audio_bytes = (audio_bytes << 8) | ubuff[2];
	audio_bytes = (audio_bytes << 8) | ubuff[1];
	audio_bytes = (audio_bytes << 8) | ubuff[0];
	
	printf ("\nTotal Bytes : %d", audio_bytes);
	file_path = new char[strlen(path)+1];
	strcpy(file_path, path);
	return 1;

open_error:
	close();
	return 0;
}

void Wave_file::close()
{
	if (-1 != file)
	{
		::close(file); 
		file = -1;
	}
	delete [] file_path; 
	file_path = 0;
}

char *Wave_file::find_chunk(const char *buffer, const char *chunk_data, int size)
{
	bool chunk_found;
	char *end  = (char*)buffer+size;

	while (buffer < end)
	{
		if (*buffer == *chunk_data)       // found match for first char
		{
			chunk_found = 1;
			for (int i = 1 ; chunk_data[i] != 0 ; i++)
			{
				chunk_found = (chunk_found ? (buffer[i] == chunk_data[i]) : 0) ;
			}
			if (chunk_found)
			{
				return (char*)buffer;
			}
		}
		buffer++;
	}
	return 0;
}

unsigned long Wave_file::read(char *buffer, unsigned long size)
{
	return ::read(file, buffer, size);
}

