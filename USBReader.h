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

class USBReader: public HIDUniversal
{
public:
	USBReader();

	void setup();
	void loop();

protected:
    virtual void ParseHIDData(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) override;

private:
	USB _Usb;
};

#endif //_USB_READER_H_