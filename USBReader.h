/*
 * USBReader.h - Read data from USB mouse
 *
 * Written by LiuLiu.mz, March 2020.
 *
 */

#ifndef _USB_READER_H_
#define _USB_READER_H_

#include <hiduniversal.h>
#include "MacroDef.h"

#pragma pack(1)

// the data struct of my usb mouse, get it with the help of usblyzer
struct USBMouseData
{
	struct
	{
		unsigned char _left_btn : 1; // 1 byte
		unsigned char _right_btn : 1;
		unsigned char _middle_btn : 1;
		unsigned char _dummy : 5;
	};
	int _delta_x; // 2 byte
	int _delta_y;
	char _delta_z;
};

typedef void (*USBMouseData_Callback)(USBMouseData*);

class USBReader: public HIDUniversal
{
public:
	USBReader(USBMouseData_Callback data_callback);

	void setup();
	void loop();

protected:
    virtual void ParseHIDData(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) override;

private:
	USB _usb;
	USBMouseData_Callback _data_callback;
};

#endif //_USB_READER_H_