#ifndef _usb_keyboard_
#define _usb_keyboard_

#include <stm32f7xx.h>
#include <stdio.h>
#include "usbh_conf.h"
#include "stm32f769i_discovery.h"
#include <usbh_core.h>
#include <usbh_hid.h>
#include <usbh_hid_parser.h>

typedef enum {
	HID_DEMO_IDLE = 0,
	HID_DEMO_WAIT,
	HID_DEMO_START,
	HID_DEMO_MOUSE,
	HID_DEMO_KEYBOARD,
	HID_DEMO_REENUMERATE,
}HID_Demo_State;

typedef enum {
	HID_MOUSE_IDLE = 0,
	HID_MOUSE_WAIT,
	HID_MOUSE_START,
}HID_mouse_State;

typedef enum {
	HID_KEYBOARD_IDLE = 0,
	HID_KEYBOARD_WAIT,
	HID_KEYBOARD_START,
}HID_keyboard_State;

typedef struct _DemoStateMachine {
	__IO HID_Demo_State     state;
	__IO HID_mouse_State    mouse_state;
	__IO HID_keyboard_State keyboard_state;
	__IO uint8_t            select;
	__IO uint8_t            lock;
}HID_DEMO_StateMachine;

typedef enum {
	APPLICATION_IDLE = 0,
	APPLICATION_DISCONNECT,
	APPLICATION_START,
	APPLICATION_READY,
	APPLICATION_RUNNING,
}HID_ApplicationTypeDef;

// Exported functions

// static void HID_MenuInit(void);
// void HID_MenuInit(void);
void HID_MenuProcess(void);

// from stm32f7xx_it.h
#ifdef USE_USB_FS
void OTG_FS_IRQHandler(void);
#else
void OTG_HS_IRQHandler(void);
#endif

#endif
