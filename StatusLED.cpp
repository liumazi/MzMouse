/*
 * StatusLED.cpp - Show device working status by LED
 *
 * Written by LiuLiu.mz, April 2020.
 *
 */

#include <Arduino.h>
#include "StatusLED.h"

#define LED_R_PIN 5 // busy
#define LED_G_PIN 6 // idle
#define LED_B_PIN 9 // sleep

#define KEEP_BUSY_TIME 500
#define KEEP_IDLE_TIME 15000

#define MAX_PWM_R 80
#define MAX_PWM_G 30
#define MAX_PWM_B 70

StatusLED::StatusLED():
_status(DeviceStatus_Idle),
_last_loop(0),
_delta_accum(0),
_idle_accum(0),
_brightness(0),
_positive_dir(true),
_fading(false)
{
}

void StatusLED::setup()
{
	pinMode(LED_R_PIN, OUTPUT);
	analogWrite(LED_R_PIN, 0);

	pinMode(LED_G_PIN, OUTPUT);
	analogWrite(LED_G_PIN, 0);

	pinMode(LED_B_PIN, OUTPUT);
	analogWrite(LED_B_PIN, 0);

	_last_loop =  millis();
}

void StatusLED::loop()
{
	unsigned long now = millis();
	unsigned long delta = now - _last_loop;
	_last_loop = now;

	switch (_status)
	{
	case DeviceStatus_Busy:
		// try turn into idle status
		_idle_accum += delta;
		if (_idle_accum > KEEP_BUSY_TIME && !_positive_dir)
		{
			_status = DeviceStatus_Idle;
		//	_last_loop = millis();
			_delta_accum = 0;
			_idle_accum = 0;
			_brightness = 0;
			_positive_dir = false; // used after faded
			_fading = true; // red -> green

			analogWrite(LED_R_PIN, MAX_PWM_R);
			analogWrite(LED_G_PIN, 0);
			analogWrite(LED_B_PIN, 0);
		}
		else
		{
			// update red light on/off
			_delta_accum += delta;
			if (_delta_accum >= 40)
			{
				_delta_accum = 0;

				analogWrite(LED_R_PIN, _positive_dir ? MAX_PWM_R : 0);

				_positive_dir = !_positive_dir;
			}
		}
		break;

	case DeviceStatus_Idle:
		// try turn into sleep status
		_idle_accum += delta;
		if (_idle_accum > KEEP_IDLE_TIME && !_positive_dir && _brightness >= MAX_PWM_G)
		{
			_status = DeviceStatus_Sleep;
		//	_last_loop = millis();
			_delta_accum = 0;
			_idle_accum = 0;
			_brightness = 0;
			_positive_dir = false; // used after faded
			_fading = true; // green -> blue

			analogWrite(LED_R_PIN, 0);
			analogWrite(LED_G_PIN, MAX_PWM_G);
			analogWrite(LED_B_PIN, 0);
		}
		else
		{
			// update green light brightness
			_delta_accum += delta;
			if (_delta_accum >= 80 - 60 * _brightness / MAX_PWM_G)
			{
				_delta_accum = 0;

				if (_fading) // red -> green
				{
					_brightness++;

					analogWrite(LED_R_PIN, MAX_PWM_R - MAX_PWM_R * _brightness / MAX_PWM_G);
					analogWrite(LED_G_PIN, _brightness);

					if (_brightness >= MAX_PWM_G)
					{
						_fading = false;
					}
				}
				else
				{
					if (_positive_dir)
					{
						_brightness++;
						if (_brightness >= MAX_PWM_G)
						{
							_positive_dir = false;
						}
					}
					else
					{
						_brightness--;
						if (_brightness == 0)
						{
							_positive_dir = true;
						}
					}
					
					analogWrite(LED_G_PIN, _brightness);
				}
			}
		}
		break;

	case DeviceStatus_Sleep:
		// update blue light brightness
		_delta_accum += delta;
		if (_delta_accum >= 60 - 55 * _brightness / MAX_PWM_B)
		{
			_delta_accum = 0;

			if (_fading) // green -> blue
			{
				_brightness++;

				analogWrite(LED_G_PIN, MAX_PWM_G - MAX_PWM_G * _brightness / MAX_PWM_B);
				analogWrite(LED_B_PIN, _brightness);

				if (_brightness >= MAX_PWM_B)
				{
					_fading = false;
				}
			}
			else
			{
				if (_positive_dir)
				{
					_brightness++;
					if (_brightness >= MAX_PWM_B)
					{
						_positive_dir = false;
					}
				}
				else
				{
					_brightness--;
					if (_brightness == 0)
					{
						_positive_dir = true;
					}
				}

				analogWrite(LED_B_PIN, _brightness);
			}
		}
		break;
	}
}

void StatusLED::busy()
{
	if (DeviceStatus_Busy == _status)
	{
		_idle_accum = 0;
	}
	else
	{
		_status = DeviceStatus_Busy;
		_last_loop = millis();
		_delta_accum = 0;
		_idle_accum = 0;
	//	_brightness = 0;
		_positive_dir = true;
	//	_fading = false;

		digitalWrite(LED_R_PIN, LOW);
		analogWrite(LED_G_PIN, 0);
		analogWrite(LED_B_PIN, 0);
	}
}
