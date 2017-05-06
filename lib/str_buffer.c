#include "str_buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void str_buffer_init(str_buffer_t* buf, uint16_t len)
{
	buf->contents = malloc(sizeof(char) * len+1);
	if( buf->contents == NULL){
		// error
	}
	buf->contents[len] = 0; // null terminate
	buf->ix_read = 0;
	buf->ix_write = 0;
	buf->length = len; // count of chars
	buf->room = len-1; // how much space left in buf
}
/* 	notes:
		if a new string to be enqueued doesn't fit, it will be dropped
	*/
void str_buffer_enqueue(str_buffer_t* buf, char* s)
{
	uint16_t i;
	uint16_t len = strlen(s) + 1; // add 1 to include \0
	if(len < buf->room){ // check there's enough space left
		uint16_t end_ix = len + buf->ix_write; // non-wrapped end
		if(end_ix < buf->length){ // no wrap
			for(i=0; i<len; i++){
				buf->contents[buf->ix_write++] = *s++;
			}
		} else { // string rolls over
			while(buf->ix_write < len){
				buf->contents[buf->ix_write++] = *s++;
			}
			buf->ix_write = 0; // wrap
			for(i=0; i<(end_ix - len); i++){
				buf->contents[buf->ix_write++] = *s++;
			}
		}
		buf->room -= len; // decrease room counter
	}
}
char* str_buffer_dequeue(str_buffer_t* buf)
{
	char* retval = buf->contents + buf->ix_read; // mem address
	buf->ix_read += strlen(&buf->contents[buf->ix_read]) + 1;
	if(buf->ix_read >= buf->length - 1) {buf->ix_read = 0;}
	return retval;
}
uint8_t str_buffer_notempty(str_buffer_t* buf)
{
	return (buf->ix_read == buf->ix_write);
}