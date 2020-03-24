/*
 * MzMouse.ino - Convert USB mouse to PS/2 mouse.
 *
 * Written by LiuLiu.mz, March 2020.
 *
 */

#include "PS2Mouse.h"
#include "USBReader.h"

PS2Mouse ps2_mouse;
USBReader usb_reader(&on_usb_data);

void on_usb_data(USBMouseData_Rapoo1680* data)
{
	// TODO: _delta_x or _delta_y out of range
	ps2_mouse.sample(PS2MouseSample(data->_left_btn, data->_right_btn, data->_middle_btn, data->_delta_x, -data->_delta_y, -data->_delta_z));
#ifdef MZ_MOUSE_DEBUG
	Serial.print(data->_left_btn);
	Serial.print(", ");
	Serial.print(data->_right_btn);
	Serial.print(", ");
	Serial.print(data->_middle_btn);
	Serial.print(", ");
	Serial.print(data->_delta_x, HEX);
	Serial.print(", ");
	Serial.print(data->_delta_y, HEX);
	Serial.print(", ");
	Serial.print(data->_delta_z, HEX);
	Serial.println(".");
#endif
}

void setup()
{
#ifdef MZ_MOUSE_DEBUG
	Serial.begin(115200);
#endif
	usb_reader.setup();
	ps2_mouse.setup();
}

void loop()
{
	usb_reader.loop();
	ps2_mouse.loop();
}