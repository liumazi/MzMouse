#include "PS2Mouse.h"

ps2mouse ps2mouse;

void setup()
{
	ps2mouse.setup();
}

void loop()
{
	ps2mouse.loop();
	ps2mouse.sample(ps2mouse_sample(0, 0, 0, 1, 1));

	delay(50); // TODO: delta
}