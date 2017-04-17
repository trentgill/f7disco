#ifndef _disco_term_
#define _disco_term_

#include <string.h>
#include <stdio.h>

#define TERM_MAX_LINES	10
#define TERM_CHARS_P_L	51

void Disco_Term_Splash(void);
void Disco_Term_Printf(unsigned char* s);
void Disco_Term_CL(unsigned char* s);
void Disco_Term_PrintLine(unsigned char* s);

// REPL functions: read/eval/print/loop
void Disco_Term_Read_String(unsigned char* s);
void Disco_Term_Read_Char(unsigned char c);
void Disco_Term_Read_Backspace(void);
void Disco_Term_Read_Clear(void);
void Disco_Term_Read_Debug(unsigned char* s);
unsigned char* Disco_Term_Eval(void);

#endif