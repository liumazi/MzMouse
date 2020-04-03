/*
 * PS2Mouse.cpp - Emulate a PS/2 mouse.
 *
 * Written by LiuLiu.mz, March 2020.
 *
 */

#include "PS2Mouse.h"

PS2MouseSample::PS2MouseSample():
_left_btn(0),
_right_btn(0),
_middle_btn(0),
_delta_x(0),
_delta_y(0),
_delta_z(0)
{
}

PS2MouseSample::PS2MouseSample(char left_btn, char right_btn, char middle_btn, int delta_x, int delta_y, char delta_z):
_left_btn(left_btn),
_right_btn(right_btn),
_middle_btn(middle_btn),
_delta_x(delta_x),
_delta_y(delta_y),
_delta_z(delta_z)
{
}

bool PS2MouseSample::merge(const PS2MouseSample& other)
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

	// impossible to overflow, unless that two values are very large or very samll
	int delta_x = _delta_x + other._delta_x;
	if (delta_x > 255 || delta_x < -255)
	{
		return false;
	}

	// impossible to overflow, unless that two values are very large or very samll
	int delta_y = _delta_y + other._delta_y;
	if (delta_y > 255 || delta_y < -255)
	{
		return false;
	}

	// impossible to overflow, unless that two values are very large or very samll
	char delta_z = _delta_z + other._delta_z;
	if (delta_z > 7 || delta_z < -7)
	{
		return false;
	}

	_delta_x = delta_x;
	_delta_y = delta_y;
	_delta_z = delta_z;
	return true;
}

void PS2MouseSample::clear()
{
	_delta_x = _delta_y = _delta_z = 0;
}

#define PS2_MOUSE_CLK_PIN 3
#define PS2_MOUSE_DATA_PIN 2

PS2Mouse::PS2Mouse():
PS2dev(PS2_MOUSE_CLK_PIN, PS2_MOUSE_DATA_PIN),
_mode(PS2MouseMode_Reset),
_sample_rate(100),
_resolution(2),
_scaling(0),
_enable(0), // we start off not enabled
_last_mode(PS2MouseMode_Reset),
_sample_queue(),
_last_sent_sample()
{
}

void PS2Mouse::setup()
{
	// send the mouse start up
	write(0xAA);
	write(0x00);
}

void PS2Mouse::loop()
{
	if (digitalRead(PS2_MOUSE_CLK_PIN) == LOW || digitalRead(PS2_MOUSE_DATA_PIN) == LOW)
	{
		unsigned char cmd, n = 0;
		/*
		while (n < 2 && read(&cmd)) // TODO: endless loop ??
		{
			n++;
		}

		if (n < 2)
		{
			process_cmd(cmd);
		}
		*/

		if (read(&cmd) == 0)
		{
			process_cmd(cmd);
		}
	}

	if (_enable)
	{
		int n = _sample_queue.len(); // (_sample_queue.len() + 1) / 2;
		while (n-- > 0 && send_movement());
	}
}

void PS2Mouse::sample(const PS2MouseSample& sample)
{
	if (_mode == PS2MouseMode_Stream || _mode == PS2MouseMode_Remote)
	{
		PS2MouseSample* last_sample = _sample_queue.tail();

		if (!last_sample || !last_sample->merge(sample))
		{
			_sample_queue.put(sample);
		}
	}
}

// acknowledge a host command
void PS2Mouse::send_ack()
{
	while (write(0xFA)); // try to write until successful, TODO: endless loop ??
}

// write a movement(and button) info packet
bool PS2Mouse::send_movement()
{
	char overflow_x, overflow_y;
	int fixed_x, fixed_y, fixed_z;
	unsigned char byte_0;

	PS2MouseSample* sample = _sample_queue.head();
	if (!sample)
	{
		return false;
	}

	// packet data begin

	// the movement counters are 9-bit 2's complement integers
	// if this range(-255 ~ 255) is exceeded, the counter is not inc/dec until it is reset

	// fix delta_x to 9bit (-256 is invaild ?)
	if (sample->_delta_x > 255)
	{
		overflow_x = 1;
		fixed_x = 255;
	}
	else
		if (sample->_delta_x < -255)
		{
			overflow_x = 1;
			fixed_x = -255;
		}
		else
		{
			overflow_x = 0;
			fixed_x = sample->_delta_x;
		}

	// fix delta_y to 9bit (-256 is invaild ?)
	if (sample->_delta_y > 255)
	{
		overflow_y = 1;
		fixed_y = 255;
	}
	else
		if (sample->_delta_y < -255)
		{
			overflow_y = 1;
			fixed_y = -255;
		}
		else
		{
			overflow_y = 0;
			fixed_y = sample->_delta_y;
		}

	// fix delta_z to 4bit (-8 ~ 7)
	if (sample->_delta_z > 7)
	{
		fixed_z = 7;
	}
	else
		if (sample->_delta_z < -8)
		{
			fixed_z = -8;
		}
		else
		{
			fixed_z = sample->_delta_z;
		}
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
	byte_0 =
		(overflow_y << 7) |
		(overflow_x << 6) |
		(fixed_y >> 3 & 0x20) |
		(fixed_x >> 4 & 0x10) |
		(8) |
		(sample->_middle_btn << 2) |
		(sample->_right_btn << 1) |
		(sample->_left_btn << 0);
	/*
	data[1] = fixed_x & 0xFF;
	data[2] = fixed_y & 0xFF;
	data[3] = fixed_z & 0xFF;
	*/

	if (!write(byte_0) && !write(fixed_x) && !write(fixed_y) && !write(fixed_z))
	{
		_last_sent_sample = *sample;
		_sample_queue.discard();
		return true;
	}
	else
	{
		return false;
	}
}

void PS2Mouse::send_status()
{
	unsigned char byte_0 =
		//(0 << 7) |
		(_mode == PS2MouseMode_Remote ? 4 : 0) |
		(_enable << 5) |
		(_scaling << 4) |
		//(0 << 3)
		(_last_sent_sample._left_btn << 2) |
		(_last_sent_sample._middle_btn << 1) |
		(_last_sent_sample._right_btn);

	write(byte_0);
	write(_resolution);
	write(_sample_rate);
}

void PS2Mouse::process_cmd(int command)
{
#ifdef MZ_MOUSE_DEBUG
	Serial.print("process_cmd ");
	Serial.println(command, HEX);
#endif

	unsigned char value;

	// This implements enough mouse commands to get by, most of them are
	// just acked without really doing anything

	switch (command)
	{
	case 0xFF: // reset
		// _mode = PS2MouseMode_Reset;
		send_ack();

		// the while loop lets us wait for the host to be ready, TODO: endless loop ??
		while (write(0xAA)); // BAT successful  
		while (write(0x00)); // device ID, no extended mouse

		_last_mode = PS2MouseMode_Stream;
		_mode = PS2MouseMode_Stream;
		_sample_rate = 100;
		_resolution = 2;
		_scaling = 0;
		_enable = 0;
		_sample_queue.clear();
		break;

	case 0xFE: // resend, host receives invalid data form mouse
		send_ack(); // responds the last packet, movenment / status / completion code id (AA 00) / ACK
		break;

	case 0xF6: // set defaults    
		send_ack();

		// enter stream mode ??
		// Sampling rate = 100
		// Resolution = 4 counts/mm
		// Scaling = 1:1
		// Disable Data Reporting

		_last_mode = PS2MouseMode_Stream;
		_mode = PS2MouseMode_Stream;
		_sample_rate = 100;
		_resolution = 2;
		_scaling = 0;
		_enable = 0;
		_sample_queue.clear();
		break;

	case 0xF5:  // disable data reporting
		send_ack();

		_enable = 0; // stream mode + disable report = remote mode
		_sample_queue.clear();
		break;

	case 0xF4: // enable data reporting
		send_ack();

		_enable = 1;
		_sample_queue.clear();
		break;

	case 0xF3: // set sample rate
		send_ack();
		read(&value); // for now drop the new rate on the floor, while in read() inside
		send_ack();
#ifdef MZ_MOUSE_DEBUG
		Serial.print("set sample rate ");
		Serial.println(value, DEC);
#endif
		_sample_rate = value;
		_sample_queue.clear();
		break;

	case 0xF2: // get device id
		send_ack();
		write(0x03); // mouse with wheel, TODO: why not while ??

		_sample_queue.clear();
		break;

	case 0xF0: // set remote mode 
		send_ack();

		_last_mode = PS2MouseMode_Remote;
		_mode = PS2MouseMode_Remote;
		_sample_queue.clear();
		break;

	case 0xEE: // set wrap mode
		send_ack();

		_mode = PS2MouseMode_Wrap;
		_sample_queue.clear();
		break;

	case 0xEC: // reset wrap mode
		send_ack();

		_mode = _last_mode;
		_sample_queue.clear();
		break;

	case 0xEB: // read data. if (_mode == PS2MouseMode_Remote) ..
		send_ack();
		send_movement(); // not while ??

		_sample_queue.clear();
		break;

	case 0xEA: // set stream mode
		send_ack();

		_last_mode = PS2MouseMode_Stream;
		_mode = PS2MouseMode_Stream;
		_sample_queue.clear();
		break;

	case 0xE9: // status request
		send_ack();
		send_status(); // not while ??

		_sample_queue.clear();
		break;

	case 0xE8: // set resolution
		send_ack();
		read(&value);
		send_ack();
#ifdef MZ_MOUSE_DEBUG
		Serial.print("set resolution ");
		Serial.println(value, DEC);
#endif
		_resolution = value;
		_sample_queue.clear();
		break;

	case 0xE7: // set scaling 2:1
		send_ack();

		_scaling = 1;
		_sample_queue.clear();
		break;

	case 0xE6: // set scaling 1:1
		send_ack();

		_scaling = 0;
		_sample_queue.clear();
		break;

	/*
	else
		resend-0xFE Error-0xFC
	*/
	}
}
