#include <stm32f769i_discovery_lcd.h>
#include "disco_term.h"
#include "debug_usart.h"

extern DSI_HandleTypeDef hdsi_discovery;

typedef struct term {
	unsigned char 	line[TERM_MAX_LINES][TERM_CHARS_P_L];
	unsigned char 	prompt[TERM_CHARS_P_L]; // << sloppy
	uint8_t 		ix_read;
	// uint8_t			ix_len;
} term_t;

term_t 	dterm;

void Disco_Term_Splash(void)
{
	BSP_LCD_SetTextColor(LCD_COLOR_GRAY);
	BSP_LCD_FillRect(0,  0, 800,  72);

	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	BSP_LCD_FillRect(0, 72, 800, 408);

	HAL_DSI_Refresh(&hdsi_discovery);

	BSP_LCD_SetFont(&Font24);
	BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);  // white on blue
}

// REPL

// Private Functions
void Disco_Term_Draw_Prompt( void )
{
	BSP_LCD_ClearStringLine(19);
	BSP_LCD_DisplayStringAtLine(19, dterm.prompt );
	HAL_DSI_Refresh(&hdsi_discovery);
}

// Public Definitions 
void Disco_Term_Read_String(unsigned char* s)
{
	strcpy( dterm.prompt, "> " );
	strcat( dterm.prompt, s );

	Disco_Term_Draw_Prompt();
}

void Disco_Term_Read_Char(unsigned char c)
{
	// leave space for >, " ", <new char>, and \0
	if( strlen(dterm.prompt) < TERM_CHARS_P_L-4 ){
		strncat( dterm.prompt, &c, 1 ); // append single char
	} else { ;; } // data entry error

	Disco_Term_Draw_Prompt();
}

void Disco_Term_Read_Backspace(void)
{
	 // set last non-null char to null
	size_t tlen = strlen(dterm.prompt);
	if( tlen > 2 ){ dterm.prompt[tlen-1] = '\0'; }

	Disco_Term_Draw_Prompt();
}

void Disco_Term_Read_Clear(void)
{
	strcpy( dterm.prompt, "> " );

	Disco_Term_Draw_Prompt();
}

void Disco_Term_Read_Debug(unsigned char* s)
{
	// Display the new line at bottom
	BSP_LCD_SetBackColor(LCD_COLOR_GRAY);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

	// Use top line only
	BSP_LCD_ClearStringLine(1);
	BSP_LCD_DisplayStringAtLine(1, s);
	HAL_DSI_Refresh(&hdsi_discovery);

	// Reset colours for standard output
	BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
}

unsigned char* Disco_Term_Eval(void)
{
	uint8_t i;

	// currently just a ring buffer (ie always full)
	for( i=0; i<(strlen(dterm.prompt)-2); i++) { // ignore leading "> "
		dterm.line[dterm.ix_read][i] = dterm.prompt[i+2];
	} dterm.line[dterm.ix_read][i] = '\0'; // null terminate string (???)

	int8_t row = dterm.ix_read;
	for( i=0; i<(TERM_MAX_LINES-2); i++ ){
		uint8_t tum = 17 - (i*2); // 1st line above prompt

		BSP_LCD_ClearStringLine(tum);
		BSP_LCD_DisplayStringAtLine(tum, dterm.line[row]);

		row--; if( row<0 ){ row += TERM_MAX_LINES; }
	}
	Disco_Term_Read_Clear(); // clear prompt & redraw screen

	dterm.ix_read++;
	if( dterm.ix_read > (TERM_MAX_LINES-1) ) { dterm.ix_read = 0; }

	return dterm.line[dterm.ix_read];
}
