/*
 * ps2mouse.h - a library to emulate ps2 mouse.
 *
 * Modified by LiuLiu.mz, March 2020.
 *
 */

#include "ps2dev.h"

mouse_sample()::mouse_sample():
_left_btn(0),
_right_btn(0),
_middle_btn(0),
_delta_x(0),
_delta_y(0)
{
}

mouse_sample::mouse_sample(char left_btn, right_btn, middle_btn, int dalta_x, delta_y):
_left_btn(left_btn),
_right_btn(right_btn),
_middle_btn(middle_btn),
_delta_x(dalta_x),
_delta_y(delta_y)
{
}

bool mouse_sample::merge(const ps2mouse_sample& other);
{
	if (_left_btn != other._left_btn)
	{
		return false;
	}

	if (_right_btn != other._right_btn)
	{
		return false;
	}

	if (_middle_btn != other._middle_btn)
	{
		return false;
	}

	int delta_x = _delta_x + other._delta_x;
	if (_delta_x > 0 && other._delta_x > 0 && delta_x < 0)
	{
		return false;
	}
	if (_delta_x < 0 && other._delta_x < 0 && delta_x >= 0)
	{
		return false;
	}

	int delta_y = _delta_y + other._delta_y;
	if (_delta_y > 0 && other._delta_y > 0 && delta_y < 0)
	{
		return false;
	}
	if (_delta_y < 0 && other._delta_y < 0 && delta_y >= 0)
	{
		return false;
	}

	_delta_x = delta_x;
	_delta_y = delta_y;
	return true;
}

void mouse_sample::clear()
{
	_dalta_x, _delta_y = 0;
}

#define MOUSE_CLK_PIN 3
#define MOUSE_DATA_PIN 2

ps2mouse::ps2mouse():
ps2dev(MOUSE_CLK_PIN, MOUSE_DATA_PIN),
_last_mode(mouse_mode_reset),
_mode(mouse_mode_reset),
_sample_rate(100),
_resolution(2),
_scaling(0),
_enable(0), // we start off not enabled
_sample_queue(),
_picketing(false)
{
}

// acknowledge a host command
void ps2mouse::write_ack()
{
	while (write_byte(0xFA)); // try to write until successful (may cause endless loop ??)
}

// write a movement(and button) info packet
void ps2mouse::write_movement()
{
	char overflow_x, overflow_y;
	int fixed_x, fixed_y;
	unsigned char data[3];

	ps2mouse_sample* sample = _sample_queue.head();
	if (!sample)
	{
		return;
	}

	// packet data begin
	// _picketing = true;

	// the movement counters are 9-bit 2's complement integers
	// if this range(-255 ~ 255) is exceeded, the counter is not inc/dec until it is reset

	// fix delta_x to 9bit (-256 is invaild ?)
	if (sample._delta_x > 255)
	{
		overflow_x = 1;
		fixed_x = 255;
	}
	else
		if (sample._delta_x < -255)
		{
			overflow_x = 1;
			fixed_x = -255;
		}
		else
		{
			overflow_x = 0;
			fixed_x = _delta_x;
		}

	// fix delta_y to 9bit (-256 is invaild ?)
	if (sample._delta_y > 255)
	{
		overflow_y = 1;
		fixed_y = 255;
	}
	else
		if (sample.delta_y < -255)
		{
			overflow_y = 1;
			fixed_y = -255;
		}
		else
		{
			overflow_y = 0;
			fixed_y = delta_y;
		}

	data[0] =
		(overflow_y << 7) |
		(overflow_x << 6) |
		(fixed_y >> 3 & 0x20) |
		(fixed_x >> 4 & 0x10) |
		(8) |
		(sample._middle_btn << 2) |
		(sample._right_btn << 1) |
		(sample._left_btn << 0);

	/*
	data[0] =
	( (overflowy & 1) << 7) |
	( (overflowx & 1) << 6) |
	( (((delta_y &0x100)>>8) & 1) << 5) |
	( ( ((delta_x &0x100)>>8)& 1) << 4) |
	( ( 1) << 3) |
	( ( buttons[1] & 1) << 2) |
	( ( buttons[2] & 1) << 1) |
	( ( buttons[0] & 1) << 0) ;
	*/

	// packet data end
	// _picketing = false;

	data[1] = fixed_x & 0xFF;
	data[2] = fixed_y & 0xFF;

	if (!write_byte(data[0]) && !write_byte(data[1]) && !write_byte(data[2]))
	{
		// sample._delta_x = 0;
		// sample._delta_y = 0;
		// _sample_queue.get();
		_sample_queue.discard();
	}
	else
	{
		// put sample to _sample_queue head
		// _sample_queue.set(sample);
	}
}

void ps2mouse::write_status()
{
	unsigned char byte1 =
		//(0 << 7) |
		(_mode == mouse_mode_remote ? 4 : 0) |
		(_enable << 5) |
		(_scaling << 4) |
		//(0 << 3)
		(_left_btn << 2) |
		(_middle_btn << 1) |
		(_right_btn);

	write_byte(byte1);
	write_byte(_resolution);
	write_byte(_sample_rate);
}

void ps2mouse::process_cmd(int command)
{
	unsigned char value;

	// This implements enough mouse commands to get by, most of them are
	// just acked without really doing anything

	switch (command)
	{
	case 0xFF: // reset
		_mode = mouse_mode_reset;

		write_ack();

		// the while loop lets us wait for the host to be ready
		while (write_byte(0xAA)); // BAT successful  
		while (write_byte(0x00)); // device ID, no extended mouse

		_last_mode = mouse_mode_stream;
		_mode = mouse_mode_stream;
		_sample_rate = 100;
		_resolution = 2;
		_scaling = 0;
		_enable = 0;
		_picketing = false;

		_sample_queue.clear();
		break;

	case 0xFE: // resend, host receives invalid data form mouse
		write_ack(); // responds the last packet, movenment / status / completion code id (AA 00) / ACK
		break;

	case 0xF6: // set defaults    
		write_ack();

		// enter stream mode ??
		// Sampling rate = 100
		// Resolution = 4 counts/mm
		// Scaling = 1:1
		// Disable Data Reporting

		_last_mode = mouse_mode_stream;
		_mode = mouse_mode_stream;
		_sample_rate = 100;
		_resolution = 2;
		_scaling = 0;
		_enable = 0;

		_sample_queue.clear();
		break;

	case 0xF5:  // disable data reporting
		write_ack();

		_enable = 0; // stream mode + disable report = remote mode
		_sample_queue.clear();
		break;

	case 0xF4: // enable data reporting
		write_ack();

		_enable_report = 1;
		_sample_queue.clear();
		break;

	case 0xF3: // set sample rate
		write_ack();
		read_byte(&value); // for now drop the new rate on the floor, while in read_byte() inside

#ifdef DEBUG
		Serial.print("set sample rate ");
		Serial.println(value, HEX);
#endif
		_sample_rate = value;

		write_ack();

		_sample_queue.clear();
		break;

	case 0xF2: // get device id
		write_ack();
		while (write_byte(0x00)); // mouse.write_byte(00);

		_sample_queue.clear();
		break;

	case 0xF0: // set remote mode 
		write_ack();

		_last_mode = mouse_mode_remote;
		_mode = mouse_mode_remote;

		_sample_queue.clear();
		break;

	case 0xEE: // set wrap mode
		write_ack();

		_mode = mouse_mode_wrap;

		_sample_queue.clear();
		break;

	case 0xEC: // reset wrap mode
		write_ack();

		_mode = _last_mode;

		_sample_queue.clear();
		break;

	case 0xEB: // read data. if (_mode == mouse_mode_remote) ..
		write_ack();
		write_movement(); // not while ??

		_sample_queue.clear();
		break;

	case 0xEA: // set stream mode
		write_ack();

		_last_mode = mouse_mode_stream;
		_mode = mouse_mode_stream;

		_sample_queue.clear();
		break;

	case 0xE9: // status request
		write_ack();
		write_status(); // not while ??

		_sample_queue.clear();
		break;

	case 0xE8: // set resolution
		write_ack();
		read_byte(&value);

#ifdef DEBUG
		Serial.print("set resolution ");
		Serial.println(value, HEX);
#endif
		_resolution = value;

		write_ack();

		_sample_queue.clear();
		break;

	case 0xE7: // set scaling 2:1
		write_ack();

		_scaling = 1;

		_sample_queue.clear();
		break;

	case 0xE6: // set scaling 1:1
		write_ack();

		_scaling = 0;

		_sample_queue.clear();
		break;

	/*
	else
		resend-0xFE Error-0xFC
	*/
	}
}

void ps2mouse::setup()
{
	// send the mouse start up
	while (write_byte(0xAA)); // while(mouse.write(0xAA)!=0);  
	while (write_byte(0x00)); // while(mouse.write(0x00)!=0);
}

void ps2mouse::loop()
{
	unsigned char cmd;

	if (digitalRead(MOUSE_CLK_PIN) == LOW || digitalRead(MOUSE_DATA_PIN) == LOW)
	{
		while (read_byte(&cmd)); // TODO: mouse_mode_wrap ..
		process_cmd(cmd);
	}

	if (_enable)
	{
		sample(ps2mouse_sample(0, 0, 0, 1, 1));
		write_movement();
	}

	delay(50); // 200sample/sec
}

void ps2mouse::sample(const ps2mouse_sample& sample)
{
	if (_mode == mouse_mode_stream || _mode == mouse_mode_remote)
	{
		ps2mouse_sample* last_sample = _sample_buff.tail();

		if (!last_sample || !last_sample.merge(sample))
		{
			_sample_buff.put(sample);
		}
	}
}