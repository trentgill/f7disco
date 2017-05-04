#include <stm32f769i_discovery_lcd.h>
#include "disco_term.h"
#include "debug_usart.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

extern DSI_HandleTypeDef hdsi_discovery;

typedef struct term {
	unsigned char 	line[TERM_MAX_LINES][TERM_CHARS_P_L];
	unsigned char 	prompt[TERM_CHARS_P_L]; // << sloppy
	uint8_t 		ix_eval;
	// uint8_t			ix_len;
} term_t;

term_t 	dterm;

void Disco_Term_Splash(void)
{
	BSP_LCD_SetTextColor(LCD_COLOR_GRAY);
	BSP_LCD_FillRect(0,  0, 800,  72);

	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	BSP_LCD_FillRect(0, 72, 800, 408);


	BSP_LCD_SetFont(&Font24);
	BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);  // white on blue

	// clear buffers
	strcpy( dterm.prompt, "> \0\0" );
	for(uint8_t i=0; i<TERM_MAX_LINES; i++){
		strcpy( dterm.line[i], " \0\0" );
	}

	HAL_DSI_Refresh(&hdsi_discovery);
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
	strcpy( dterm.prompt, "> \0" ); // rm this line
	strcat( dterm.prompt, s );

	Disco_Term_Draw_Prompt();
}

void Disco_Term_Read_Char(unsigned char c)
{
	// leave space for >, " ", <new char>, and \0
	// strncat( dterm.prompt, c, 1 ); // append single char
	/*if( strlen(dterm.prompt) < TERM_CHARS_P_L-4 ){
	} else { ;; } // data entry error
*/
	static char buf[2] = {32,0};
	buf[0] = c;
	strcat( dterm.prompt, buf );
	Disco_Term_Draw_Prompt();
}

void Disco_Term_Read_Backspace(void)
{
	 // set last non-null char to null
	size_t tlen = strlen(dterm.prompt);
	// >3 for "> " (2 chars) plus \0
	// -1 for 0reference, -2 to reach last real char
	if( tlen > 3 ){ dterm.prompt[tlen-2] = '\0'; }

	Disco_Term_Draw_Prompt();
}

void Disco_Term_Read_Clear(void)
{
	strcpy( dterm.prompt, "> \0" );
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

void Disco_Term_Redraw_History(int8_t row)
{
	for( uint8_t i=0; i<(TERM_MAX_LINES-2); i++ ){
		uint8_t line = 17 - (i*2); // count up from bottom(17)

		BSP_LCD_ClearStringLine(line);
		BSP_LCD_DisplayStringAtLine(line, dterm.line[row]);

		row--; if( row<0 ){ row += TERM_MAX_LINES; }
	}
}

unsigned char* Disco_Term_Eval(void)
{
	// format prompt into string for lua w return
	char lstring[60] = "return \0";
	char* firstchar = &(dterm.prompt[2]);
	strcat(lstring, firstchar);

	lua_State *luaTerm = luaL_newstate();
	luaL_openlibs(luaTerm);
	
	// **EVAL**
	luaL_dostring(luaTerm, lstring);
	strcpy(lstring, lua_tostring(luaTerm, -1) );
	if(strlen(lstring) > TERM_CHARS_P_L) {
		strncpy(dterm.line[dterm.ix_eval], lstring, 47);
	} else {
		strcpy(dterm.line[dterm.ix_eval], lstring);
	}

	lua_close(luaTerm);

	// redraw history, clear prompt & redraw screen
	Disco_Term_Redraw_History( (int8_t)dterm.ix_eval );
	Disco_Term_Read_Clear();

	// save ix
	uint8_t tmp = dterm.ix_eval;

	// move index into line[] buffer
	dterm.ix_eval++;
	if( dterm.ix_eval > (TERM_MAX_LINES-1) ) { dterm.ix_eval = 0; }

	// return copy of output
	return dterm.line[tmp];
}
