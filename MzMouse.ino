/*
 * MzMouse.ino - Convert USB mouse to PS/2 mouse.
 *
 * Written by LiuLiu.mz, March 2020.
 *
 */

#include "PS2Mouse.h"
#include "USBReader.h"

PS2Mouse ps2_mouse;
USBReader usb_reader;

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
	ps2_mouse.loop();
	ps2_mouse.sample(PS2MouseSample(0, 0, 0, 1, 1, -1));

	delay(50); // TODO: delta
}