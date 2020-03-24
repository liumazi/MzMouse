/*
 * USBReader.cpp - Read data from USB mouse
 *
 * Written by LiuLiu.mz, March 2020.
 *
 */

#include "USBReader.h"

USBReader::USBReader(USBMouseData_Callback data_callback):
HIDUniversal(&_usb),
_data_callback(data_callback)
{

}

void USBReader::setup()
{
	if (_usb.Init() == -1)
	{
#ifdef MZ_MOUSE_DEBUG
		Serial.println("usb init failed");
#endif
	}
}

void USBReader::loop()
{
	_usb.Task();
}

void USBReader::ParseHIDData(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf)
{
#ifdef MZ_MOUSE_DEBUG
	Serial.print("HID Data Len");
	Serial.println(len);
#endif
	if (buf != nullptr && sizeof(USBMouseData_Rapoo1680) == len && _data_callback)
	{
		_data_callback((USBMouseData_Rapoo1680*)buf);
	}
}