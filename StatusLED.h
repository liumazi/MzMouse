/*
 * StatusLED.h - Show device working status by LED
 *
 * Written by LiuLiu.mz, April 2020.
 *
 */

#ifndef _STATUS_LED_H_
#define _STATUS_LED_H_

enum DeviceStatus
{
	DeviceStatus_Busy, DeviceStatus_Idle, DeviceStatus_Sleep
};

class StatusLED
{
public:
	StatusLED();

	void setup();
	void loop();

	void busy();

private:
	DeviceStatus _status; // device status

	unsigned long _last_loop; // timestamp of the last call loop()
	unsigned long _delta_accum; // between call loop() delta time accum

	unsigned long _idle_accum; // device idle time accum (for turn status, busy to idle, idle to sleep)

	unsigned char _brightness; // led brightness (for breathing)
	bool _positive_dir; // direction of led change, true inc, false dec
	
	bool _fading;
};

#endif //_STATUS_LED_H_