/*
 * ps2mouse.h - a library to emulate ps2 mouse for arduino.
 *
 * Written by LiuLiu.mz, March 2020.
 *
 */

#ifndef ps2mouse_h
#define ps2mouse_h

#include <ps2dev.h> // https://github.com/Harvie/ps2dev
#include "MacroDef.h"
#include "RingQueue.h"

struct ps2mouse_sample
{
	ps2mouse_sample();
	ps2mouse_sample(char left_btn, char right_btn, char middle_btn, int dalta_x, int delta_y, int delta_z);

	bool merge(const ps2mouse_sample& other);
	void clear();

	char _left_btn, _right_btn, _middle_btn;
	int _delta_x, _delta_y, _delta_z; // x y z movement counter
};

enum ps2mouse_mode
{
	mouse_mode_reset, mouse_mode_stream, mouse_mode_remote, mouse_mode_wrap
};

class ps2mouse : public PS2dev
{
public:
	ps2mouse();

	void setup();
	void loop();

	void sample(const ps2mouse_sample& sample);

private:
	void send_ack();      // acknowledge a host command with 0xFA
	void send_movement(); // send a movement(and button) info packet
	void send_status();   // send mouse status packet

	void process_cmd(int cmd);

	ps2mouse_mode _mode; // mode
	char _sample_rate;   // samples/sec
	char _resolution;    // 0 == 1count/ mm, 1 == 2count/mm, 2 == 4count/mm, 3 == 8count/mm
	char _scaling;       // 0 == 1:1, 1 == 2:1
	char _enable;        // enabel report movement pack
	ps2mouse_mode _last_mode; // for reset wrap mode

	ring_queue<ps2mouse_sample, 6> _sample_queue;
	ps2mouse_sample _last_sent_sample; // for send_status()
};

#endif /* ps2mouse_h */