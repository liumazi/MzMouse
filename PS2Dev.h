/*
 * ps2dev.h - a library to interface with ps2 hosts. See comments in
 * ps2.cpp.
 * Written by Chris J. Kiick, January 2008.
 * Modified by Gene E. Scogin, August 2008.
 * Modified by LiuLiu.mz, March 2020.
 * Release into public domain.
 */

#ifndef ps2dev_h
#define ps2dev_h

#include "Arduino.h"
#include "MacroDef.h"

class ps2dev
{
public:
	ps2dev(int clk_pin, int data_pin);

protected:
	int write_byte(unsigned char data); // return 0 == no error
	int read_byte(unsigned char * ptr); // return 0 == no error

private:
	void set_high(int pin); // pin to high level
	void set_low(int pin); // pin to low level

	int _clk_pin;
	int _data_pin;
};

#endif /* ps2dev_h */