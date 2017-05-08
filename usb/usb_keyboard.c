#include "usb_keyboard.h"
#include "main.h"

#include "../lib/debug_usart.h"
#include "../lib/disco_term.h"

// private declarations
void HID_KeyboardMenuProcess(void);
static void USBH_KeybdDemo(USBH_HandleTypeDef *phost);
void USR_KEYBRD_ProcessData(uint8_t data);
void USR_KEYBRD_Init(void);

// private data
USBH_HandleTypeDef hUSBHost;
HID_ApplicationTypeDef Appli_state;// = APPLICATION_IDLE;
HID_DEMO_StateMachine hid_demo;
HCD_HandleTypeDef hhcd;

// Init Fn. Looks more like the keyhandler?
HID_KEYBD_Info_TypeDef *kbInfo;


uint8_t	prev_select = 0;

uint8_t *DEMO_KEYBOARD_menu[] = {
	(uint8_t *)"      1 - Start Keyboard / Clear                                            ",
	(uint8_t *)"      2 - Return                                                            ",
}; 

uint8_t *DEMO_MOUSE_menu[] = {
	(uint8_t *)"      1 - Start Mouse / Re-Initialize                                       ",
	(uint8_t *)"      2 - Return                                                            ",
};

uint8_t *DEMO_HID_menu[] = {
	(uint8_t *)"      1 - Start HID                                                         ",
	(uint8_t *)"      2 - Re-Enumerate                                                      ",
};


void HID_MenuGo(void)
{
	// CHEAT
	hid_demo.select = 0x80;
	Appli_state = APPLICATION_READY;
	HID_MenuProcess();
}

void HID_MenuProcess(void)
{
	switch(hid_demo.state)
	{
	case HID_DEMO_IDLE:
		// HID_SelectItem(DEMO_HID_menu, 0); 
		hid_demo.state = HID_DEMO_WAIT;
		hid_demo.select = 0;
		break;        

	case HID_DEMO_WAIT:
		if(hid_demo.select != prev_select)
		{
			prev_select = hid_demo.select;

			// HID_SelectItem(DEMO_HID_menu, hid_demo.select & 0x7F); 
			/* Handle select item */
			if(hid_demo.select & 0x80)
			{
				hid_demo.select &= 0x7F;

				switch(hid_demo.select)
				{
				case 0:
					// Debug_USART_printf("demo_start\n\r");
					hid_demo.state = HID_DEMO_START;
					break;

				case 1:
					Debug_USART_printf("reenum\n\r");
					hid_demo.state = HID_DEMO_REENUMERATE;
					break;

				default:
					break;
				}
			}
			// Debug_USART_printf("no sel\n\r");
		}
		break; 

	case HID_DEMO_START:
		if(Appli_state == APPLICATION_READY)
		{
			// Debug_USART_putn( USBH_HID_GetDeviceType(&hUSBHost) );
			if(USBH_HID_GetDeviceType(&hUSBHost) == HID_KEYBOARD)
			{
				Debug_USART_printf("keys\n\r");
				hid_demo.keyboard_state = HID_KEYBOARD_IDLE; 
				hid_demo.state = HID_DEMO_KEYBOARD;
			}
			else if(USBH_HID_GetDeviceType(&hUSBHost) == HID_MOUSE)
			{
				Debug_USART_printf("mouse \n\r");
				// hid_demo.mouse_state = HID_MOUSE_IDLE;  
				// hid_demo.state = HID_DEMO_MOUSE;
			}
		}
		else
		{
			// LCD_ErrLog("No supported HID device!\n");
			Debug_USART_printf("no supported device\n\r");
			hid_demo.state = HID_DEMO_WAIT;
		}
		break;

	case HID_DEMO_REENUMERATE:
		/* Force HID Device to re-enumerate */
		Debug_USART_printf("force reenum\n\r");
		USBH_ReEnumerate(&hUSBHost); 
		hid_demo.state = HID_DEMO_WAIT;
		break;

	case HID_DEMO_MOUSE:
		if(Appli_state == APPLICATION_READY)
		{
			Debug_USART_printf("mouse process\n\r");
			// HID_MouseMenuProcess();
			// USBH_MouseDemo(&hUSBHost);
		}
		break; 

	case HID_DEMO_KEYBOARD:
		if(Appli_state == APPLICATION_READY)  
		{
			// Called repetitively on a timer
			HID_KeyboardMenuProcess();
			USBH_KeybdDemo(&hUSBHost);
		}   
		break;

	default:
		break;
	}

	if(Appli_state == APPLICATION_DISCONNECT)
	{
		Appli_state = APPLICATION_IDLE;
		hid_demo.state = HID_DEMO_IDLE;
		hid_demo.select = 0;    
	}
}


void HID_KeyboardMenuProcess(void)
{
	// Debug_USART_printf("menuprocess\n\r");
	switch(hid_demo.keyboard_state)
	{
	case HID_KEYBOARD_IDLE:
		hid_demo.keyboard_state = HID_KEYBOARD_START;
		// HID_SelectItem(DEMO_KEYBOARD_menu, 0);   
		hid_demo.select = 0;
		prev_select = 0;
		break;

	case HID_KEYBOARD_WAIT:
			// Debug_USART_printf("key wait #1\n\r");
		

		// this is re-select?!
		if(hid_demo.select != prev_select){
			prev_select = hid_demo.select ;
			// HID_SelectItem(DEMO_KEYBOARD_menu, hid_demo.select & 0x7F);
			/* Handle select item */
			if(hid_demo.select & 0x80){
				hid_demo.select &= 0x7F;
				switch(hid_demo.select){

				case 0: 
					hid_demo.keyboard_state = HID_KEYBOARD_START;
					break;

				case 1: /* Return */
					// LCD_LOG_ClearTextZone();
					hid_demo.state = HID_DEMO_REENUMERATE;
					hid_demo.select = 0;
					break;

				default:
					break;
				}
			}
		}
		break; 

	case HID_KEYBOARD_START:
		USBH_HID_KeybdInit(&hUSBHost);
		USR_KEYBRD_Init();

		hid_demo.keyboard_state = HID_KEYBOARD_WAIT;
		break;  

	default:
		break;
	}
}

void USR_KEYBRD_Init(void)
{
	// Init the screen (ready for input!)
	Disco_Term_Splash();
}

static void USBH_KeybdDemo(USBH_HandleTypeDef *phost)
{
	HID_KEYBD_Info_TypeDef *k_pinfo; 
	char c;
	k_pinfo = USBH_HID_GetKeybdInfo(phost);
	// Debug_USART_putn(k_pinfo);
	if(k_pinfo != NULL){
		c = USBH_HID_GetASCIICode(k_pinfo);

		if(c != 0){
			// text input
			USR_KEYBRD_ProcessData(c);
		} else {
			// modifiers & arrow keys
			/*if(&k_pinfo->lshift){
				// Left Shift
			}*/
			
			// NB: keys is an array with size=6
				// if more than 1 held, need to check buf
			Disco_Term_Set_Cursor( k_pinfo->keys[0] );
		}
	}
}

void USR_KEYBRD_ProcessData(uint8_t data)
{
	Debug_USART_putn(data);
	if(data == 0x0A){ // Enter
		// EVAL!
		Disco_Term_Eval();
	} else if(data == 0x0D){ // Backspace
		Disco_Term_Read_Backspace();
	} else if(data >= 0x20){ // Normal keys
		if(data < 0x7F){
			// char
			Disco_Term_Read_Char(data);
		} else { // Delete
			// currently wrong
			Disco_Term_Read_Backspace();
		}
	}


}


// IRQ Handler
// USB-On-The-Go FS/HS global interrupt request.
#ifdef USE_USB_FS
void OTG_FS_IRQHandler(void)
#else
void OTG_HS_IRQHandler(void)
#endif
{
	HAL_HCD_IRQHandler(&hhcd);
}