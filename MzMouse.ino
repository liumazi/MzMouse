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

void on_usb_data(USBMouseData* data)
{
	ps2_mouse.sample(PS2MouseSample(data->_left_btn, data->_right_btn, data->_middle_btn, data->_delta_x, data->_delta_y, data->_delta_z));
#ifdef MZ_MOUSE_DEBUG
	Serial.print(data->_left_btn);
	Serial.print(" ");
	Serial.print(data->_right_btn);
	Serial.print(" ");
	Serial.print(data->_middle_btn);
	Serial.print(" ");
	Serial.print(data->_delta_x);
	Serial.print(" ");
	Serial.print(data->_delta_y);
	Serial.print(" ");
	Serial.print(data->_delta_z);
	Serial.println(" ");
#endif
}

void setup()
{
#ifdef MZ_MOUSE_DEBUG
	Serial.begin(115200);
#endif
	ps2_mouse.setup();
	usb_reader.setup();
}

void loop()
{
	unsigned long start_time = millis();

	usb_reader.loop();
	ps2_mouse.loop();

	unsigned long elapsed_time = millis() - start_time;

	if (elapsed_time < 50)
	{
		//delay(50 - elapsed_time);
	}
}