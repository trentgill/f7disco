#ifndef _str_buffer_
#define _str_buffer_

#include <stm32f7xx.h>

typedef struct str_buffer{
	char* contents;
	int16_t ix_read;
	int16_t ix_write;
	uint16_t length;	
	uint16_t room;
} str_buffer_t;

void str_buffer_init(str_buffer_t* buf, uint16_t len);
void str_buffer_enqueue(str_buffer_t* buf, char* s);
char* str_buffer_dequeue(str_buffer_t* buf);
uint8_t str_buffer_notempty(str_buffer_t* buf);


#endif