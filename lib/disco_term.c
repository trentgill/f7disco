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
	uint8_t 		cursor;
	// uint8_t			ix_len;
} term_t;

// private declarations
void Disco_Term_Redraw_History(int8_t row);
void Disco_Term_Draw_Prompt(void);
void Disco_Term_MoveCursorLeft( void );
void Disco_Term_MoveCursorRight( void );


// private vars
term_t 	dterm;

// function definitions
void Disco_Term_Splash(void)
{
	BSP_LCD_SetTextColor(LCD_COLOR_GRAY);
	BSP_LCD_FillRect(0,  0, 800,  72);

	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	BSP_LCD_FillRect(0, 72, 800, 408);

	BSP_LCD_SetFont(&Font24);
	BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);  // white on blue

	// set to 20% (stops HF noise / lowers power)
	BSP_LCD_SetBrightness(20);
	
	// Initialize prompt & clear history
	strcpy( dterm.prompt, "> \0\0" );
	dterm.cursor = 2; // first char after "> "
	Disco_Term_Redraw_History((int8_t)dterm.ix_eval);
	Disco_Term_Draw_Prompt();
}

// REPL

// Private Functions
void Disco_Term_Draw_Prompt( void )
{
	// base colours
	BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);  // blue on white

	// display basic string
	BSP_LCD_ClearStringLine(19);
	BSP_LCD_DisplayStringAtLine(19, dterm.prompt );
	
	// set inverted colours for cursor
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);  // blue on white
	
	// draw cursor to screen
	char cc = dterm.prompt[dterm.cursor];
	if(cc == 0){ cc = ' '; }
	BSP_LCD_DisplayChar( dterm.cursor * BSP_LCD_GetCharWidth(), LINE(19), cc );

	// redraw screen
	HAL_DSI_Refresh(&hdsi_discovery);
}

void Disco_Term_MoveCursorLeft( void )
{
	dterm.cursor--;
	if(dterm.cursor < 2){ dterm.cursor = 2; }
}
void Disco_Term_MoveCursorRight( void )
{
	dterm.cursor++;
	uint8_t len = strlen(dterm.prompt);
	if(dterm.cursor > len){ dterm.cursor = len; }
}

void Disco_Term_Set_Cursor( uint8_t keycode )
{
	switch (keycode) {
	case 0x50:
		// LEFT
		Disco_Term_MoveCursorLeft();
		Disco_Term_Draw_Prompt();
		break;
	case 0x4F:
		// RIGHT
		Disco_Term_MoveCursorRight();
		Disco_Term_Draw_Prompt();
		break;
	case 0x52:
		// UP
		Debug_USART_printf("u\n\r");
		break;
	case 0x51:
		// DOWN
		Debug_USART_printf("d\n\r");
		break;
	}
}

// Public Definitions 
void Disco_Term_Read_String(unsigned char* s)
{
	strcpy( dterm.prompt, "> \0" ); // rm this line
	strcat( dterm.prompt, s );
	dterm.cursor = strlen(dterm.prompt); // move cursor to end of string

	Disco_Term_Draw_Prompt();
}

void Disco_Term_Read_Char(unsigned char c)
{
	static char buf[2] = {32,0};
	buf[0] = c;

	// ONLY JOINS ON END: update to insert at cursor
	// strcat( dterm.prompt, buf );

	uint16_t cur_len = strlen(dterm.prompt);
	if(cur_len < TERM_CHARS_P_L){
		// insert
		uint16_t cp_len = cur_len - dterm.cursor + 1; // chars after cursor
		
		char* src = &dterm.prompt[cur_len]; // index of the null char
		char* dest = src + 1;
		for(uint16_t i=0; i<cp_len; i++){
			*dest-- = *src--; // shift chars 1 place to the right
		}
		*dest = c; // insert input into the gap
	} else {
		// error! string overflows screen!
	}

	Disco_Term_MoveCursorRight();
	Disco_Term_Draw_Prompt();
}

void Disco_Term_Read_Backspace(void)
{
	uint16_t cp_len = strlen(dterm.prompt) - dterm.cursor + 1;
	
	char* src = &dterm.prompt[dterm.cursor];
	char* dest = src-1;

	if(dterm.cursor > 2){
		for(uint16_t i=0; i<cp_len; i++){
			*dest++ = *src++;
		}	
	}

	Disco_Term_MoveCursorLeft();
	Disco_Term_Draw_Prompt();
}

void Disco_Term_Read_Clear(void)
{
	strcpy( dterm.prompt, "> \0" );
	dterm.cursor = 2;
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
