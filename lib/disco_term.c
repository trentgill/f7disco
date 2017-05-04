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
	uint8_t 		dirty;
	// uint8_t			ix_len;
} term_t;

// private declarations
void Disco_Term_Redraw_History(int8_t row);
void Disco_Term_Draw_Prompt(void);
void Disco_Term_MoveCursorLeft( void );
void Disco_Term_MoveCursorRight( void );

// lua privates
static void stackDump(lua_State* L);

// private vars
term_t 	dterm;
lua_State *luaTerm;

// function definitions
void Disco_Term_Splash(void)
{
	// INIT a lua state for use by the terminal
	luaTerm = luaL_newstate();
	luaL_openlibs(luaTerm);

	// draw base state
	BSP_LCD_SetTextColor(LCD_COLOR_GRAY);
	BSP_LCD_FillRect(0,  0, 800,  72);

	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	BSP_LCD_FillRect(0, 72, 800, 408);

	BSP_LCD_SetFont(&Font24);
	BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);  // white on blue
	
	// Initialize prompt & clear history
	strcpy( dterm.prompt, "> \0\0" );
	dterm.cursor = 2; // first char after "> "
	Disco_Term_Redraw_History((int8_t)dterm.ix_eval);
	Disco_Term_Draw_Prompt();

	Disco_Term_Read_Debug("it's a lua terminal!");
}

void Disco_Term_Destroy(void)
{
	lua_close(luaTerm);
}

void Disco_Term_Timer(void)
{
	if(dterm.dirty){
		dterm.dirty = 0;
		HAL_DSI_Refresh(&hdsi_discovery);
	}
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
	dterm.dirty = 1;
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
	uint16_t cur_len = strlen(dterm.prompt);
	if(cur_len < TERM_CHARS_P_L){
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
	dterm.dirty = 1;
}

void Disco_Term_Redraw_History(int8_t row)
{
	BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

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
	// char lstring[60] = "return \0";
	char lstring[60];
	char* firstchar = &(dterm.prompt[2]);
	strcpy(lstring, firstchar);


	lua_pushboolean(luaTerm, 1);
	lua_pushnumber(luaTerm, 444);
	lua_pushstring(luaTerm, "a word");
	lua_pushnumber(luaTerm, 0x444);
	stackDump(luaTerm);	

	/*
	int32_t error = luaL_loadstring(luaTerm, lstring) ||
					lua_pcall(luaTerm, 0,0,0);
	if (error){
		Debug_USART_printf("err\n\r");
		Disco_Term_Read_Debug(lua_tostring(luaTerm, -1));
		lua_pop(luaTerm, 1);
	} else {
		Debug_USART_putn(lua_gettop(luaTerm));
		if(lua_isnumber(luaTerm, -1)){
			Debug_USART_printf("num\n\r");
		}
		strcpy(lstring, lua_tostring(luaTerm, -1));
		Debug_USART_printf("ok\n\r");
		if(strlen(lstring) > TERM_CHARS_P_L) {
			strncpy(dterm.line[dterm.ix_eval], lstring, 47);
		} else {
			strcpy(dterm.line[dterm.ix_eval], lstring);
		}
	}*/
	// luaopen_io(luaTerm);
	
	// **EVAL**
	
	/*while (fgets(buff, sizeof(buff), stdin) != NULL) {
		error = luaL_loadbuffer(L, buff, strlen(buff), "line") ||
				lua_pcall(L, 0, 0, 0);
		if (error) {
			fprintf(stderr, "%s", lua_tostring(L, -1));
			lua_pop(L, 1);  // pop error message from the stack
		}
	}*/



/*
	luaL_dostring(luaTerm, lstring);
	strcpy(lstring, lua_tostring(luaTerm, -1) );

	if(strlen(lstring) > TERM_CHARS_P_L) {
		strncpy(dterm.line[dterm.ix_eval], lstring, 47);
	} else {
		strcpy(dterm.line[dterm.ix_eval], lstring);
	}*/

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

static void stackDump(lua_State* L)
{
	int top = lua_gettop(L);
	for(uint16_t i=1; i<=top; i++){
		int t = lua_type(L, i);
		switch(t) {
			case LUA_TSTRING:
				Debug_USART_printf("string: ");
				HAL_Delay(20);
				Debug_USART_printf(lua_tostring(L,i));
				HAL_Delay(20);
				Debug_USART_printf("\n\r");
				break;
			case LUA_TBOOLEAN:
				Debug_USART_printf("bool: ");
				HAL_Delay(20);
				Debug_USART_printf(lua_toboolean(L, i) ? "true" : "false");
				break;
			case LUA_TNUMBER:
				Debug_USART_printf("num: ");
				HAL_Delay(20);
				Debug_USART_putn(lua_tonumber(L, i));
				break;
			default:
				Debug_USART_printf("other: ");
				HAL_Delay(20);
				Debug_USART_printf(lua_typename(L, t));
				break;
		}
		HAL_Delay(30);
		Debug_USART_printf(" ");
	}
	HAL_Delay(30);
	Debug_USART_printf("\n\r");
}