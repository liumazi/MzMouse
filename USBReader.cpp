/*
 * USBReader.cpp - Read data from USB mouse
 *
 * Written by LiuLiu.mz, March 2020.
 *
 */

#include "USBReader.h"

USBReader::USBReader():
HIDUniversal(&_Usb)
{

}

void USBReader::setup()
{
	if (_Usb.Init() == -1)
	{
#ifdef MZ_MOUSE_DEBUG
		Serial.println("usb init failed");
#endif
	}
}

void USBReader::loop()
{
	_Usb.Task();
}

void USBReader::ParseHIDData(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf)
{

}