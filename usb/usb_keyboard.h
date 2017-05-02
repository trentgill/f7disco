#ifndef _usb_keyboard_
#define _usb_keyboard_

#include <stdio.h>
#include <usbh_core.h>
#include <usbh_hid.h>
#include <usbh_hid_parser.h>

USBH_HandleTypeDef hUSBHost;

// Exported functions
void HID_MenuInit(void);

#ifdef USE_USB_FS
void OTG_FS_IRQHandler(void);
#else
void OTG_HS_IRQHandler(void);
#endif

#endif
