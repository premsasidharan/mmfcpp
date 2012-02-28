#include <buffer.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	Buffer *buffer = Buffer::request(1024, 1, 10);
	Buffer* sub_buff1 = buffer->split(0, 1024, 2, 5);
	Buffer* sub_buff2 = buffer->split(0, 1024, 2, 5);
	Buffer* sub_sub_buff1 = sub_buff1->split(0, 1024, 2, 5);

	printf("\nbuffer: 0x%llx, sub_buff1: 0x%llx, sub_buff2: 0x%llx, sub_sub_buff1: 0x%llx", 
		(unsigned long long)buffer, 
		(unsigned long long)sub_buff1, 
		(unsigned long long)sub_buff2, 
		(unsigned long long)sub_sub_buff1);

	printf("\n1: ");
	buffer->print(0);
	Buffer::release(sub_buff1);
	printf("\n2: ");
	buffer->print(0);
	Buffer::release(sub_buff2);
	printf("\n3: ");
	buffer->print(0);
	Buffer::release(sub_sub_buff1);
	printf("\n4: ");
	buffer->print(0);
	Buffer::release(buffer);

	return 0;
}

