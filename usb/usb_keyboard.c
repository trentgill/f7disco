#include "usb_keyboard.h"

// Init Fn. Looks more like the keyhandler?
void HID_MenuInit(void)
{
	HID_KEYBD_Info_TypdeDef *kbInfo;
	kbInfo = USBH_HID_GetKeybdInfo(&hUSBHost);
	if( kbInfo != NULL ){
		char c = USBH_HID_GetASCIICode(kbInfo);
		if( c != 0 ){
			// send to REPL if not a NULL char
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