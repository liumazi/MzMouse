/*
 * PS2Mouse.h - Emulate a PS/2 mouse.
 *
 * Written by LiuLiu.mz, March 2020.
 *
 */

#ifndef _PS2_MOUSE_H_
#define _PS2_MOUSE_H_

#include <ps2dev.h> // https://github.com/Harvie/ps2dev
#include "MacroDef.h"
#include "RingQueue.h"

struct PS2MouseSample
{
	PS2MouseSample();
	PS2MouseSample(char left_btn, char right_btn, char middle_btn, int dalta_x, int delta_y, int delta_z);

	bool merge(const PS2MouseSample& other);
	void clear();

	char _left_btn, _right_btn, _middle_btn;
	char _delta_x, _delta_y, _delta_z; // x y z movement counter
};

enum PS2MouseMode
{
	PS2_Mouse_Mode_Reset, PS2_Mouse_Mode_Stream, PS2_Mouse_Mode_Remote, PS2_Mouse_Mode_Wrap
};

class PS2Mouse : public PS2dev
{
public:
	PS2Mouse();

	void setup();
	void loop();

	void sample(const PS2MouseSample& sample);

private:
	void send_ack();      // acknowledge a host command with 0xFA
	bool send_movement(); // send a movement(and button) info packet
	void send_status();   // send mouse status packet

	void process_cmd(int cmd);

	PS2MouseMode _mode; // mode
	char _sample_rate;   // samples/sec
	char _resolution;    // 0 == 1count/ mm, 1 == 2count/mm, 2 == 4count/mm, 3 == 8count/mm
	char _scaling;       // 0 == 1:1, 1 == 2:1
	char _enable;        // enabel report movement pack
	PS2MouseMode _last_mode; // for reset wrap mode

	RingQueue<PS2MouseSample, 16> _sample_queue;
	PS2MouseSample _last_sent_sample; // for send_status()
};

#endif // _PS2_MOUSE_H_