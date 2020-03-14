/*
 * ps2dev.cpp - an interface library for ps2 host.  
 * limitations:
 *      we do not handle parity errors.
 *      The timing constants are hard coded from the spec. Data rate is not impressive.
 *      probably lots of room for optimization.
 */

//#include "WProgram.h"
#include "ps2dev.h"

// since for the device side we are going to be in charge of the clock,
// the two defines below are how long each _phase_ of the clock cycle is
#define CLKFULL 40

// we make changes in the middle of a phase, this how long from the
// start of phase to the when we drive the data line
#define CLKHALF 20

/*
 * the clock and data pins can be wired directly to the clk and data pins
 * of the PS2 connector.  No external parts are needed.
 */
ps2dev::ps2dev(int clk_pin, int data_pin):
  _clk_pin(clk_pin),
  _data_pin(data_pin)
{
#ifdef DEBUG
  Serial.print("ps2dev ");
  Serial.print(clk_pin, HEX);
  Serial.print(" ");
  Serial.println(data_pin, HEX);
#endif
  set_high(_clk_pin); // release pins
  set_high(_data_pin);
}

/*
 * according to some code I saw, these functions will
 * correctly set the clock and data pins for
 * various conditions.  It's done this way so you don't need
 * pullup resistors.
 */
void ps2dev::set_high(int pin)
{
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
}

void ps2dev::set_low(int pin)
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

int ps2dev::write_byte(unsigned char data)
{
  unsigned char i;
  unsigned char parity = 1; // odd parity

#ifdef DEBUG
  Serial.print("write ");
  Serial.println(data, HEX);
#endif
	
  if (digitalRead(_clk_pin) == LOW)
  {
#ifdef DEBUG  
    Serial.println("write is inhibited by clk pin");
#endif    
    return 1;
  }

  if (digitalRead(_data_pin) == LOW)
  {
#ifdef DEBUG  
    Serial.println("write is inhibited by data pin");
#endif     
    return 2;
  }

  // device sends on falling clk
  // delayMicroseconds(CLKHALF); ??

  // start bit
  set_low(_data_pin); // = 0
  delayMicroseconds(CLKHALF); // data changed, 5-25us delay at least
  set_low(_clk_pin); // falling clk, host will latch data
  delayMicroseconds(CLKFULL);
  set_high(_clk_pin); //  rising clk, ready next falling clk
  delayMicroseconds(CLKHALF); // rising clk to change data, 5us delay at least

  for (i=0; i < 8; i++)
  {
    if (data & 0x01) // bit i of data
    {
      set_high(_data_pin);
    }
    else
    {
      set_low(_data_pin);
    }
    delayMicroseconds(CLKHALF); // data changed, 5-25us delay at least
    set_low(_clk_pin);	// falling clk, host will latch data
    delayMicroseconds(CLKFULL);
    set_high(_clk_pin); // rising clk, ready next falling clk
    delayMicroseconds(CLKHALF); // rising clk to change data, 5us delay at least

    parity = parity ^ (data & 0x01);
    data = data >> 1;
  }

  // parity bit
  if (parity)
  {
    set_high(_data_pin);
  } 
  else 
  {
    set_low(_data_pin);
  }
  delayMicroseconds(CLKHALF);
  set_low(_clk_pin);
  delayMicroseconds(CLKFULL);
  set_high(_clk_pin);
  delayMicroseconds(CLKHALF);

  // stop bit
  set_high(_data_pin); // = 1
  delayMicroseconds(CLKHALF);
  set_low(_clk_pin);	
  delayMicroseconds(CLKFULL);
  set_high(_clk_pin);
  delayMicroseconds(CLKHALF);

  delayMicroseconds(50); // =50us from the clk is released in sending the stop bit
  return 0;
}


int ps2dev::read_byte(unsigned char * ptr)
{
  unsigned char data = 0x00;
  unsigned char i;
  unsigned char bit = 0x01;
  
  unsigned char parity = 1;

  int ret = 0;

  // the host inhibit communication by pulling clk low for at least 100 microseconds.
  // the host apply "request-to-send" by pulling data low, then release Clock.
  
  // wait for data line to go low (request-to-send)
  while (digitalRead(_data_pin) == HIGH) // start bit == 0 ??
  {
  }
  // wait for clk line to go high (inhibit communication end)
  while (digitalRead(_clk_pin) == LOW)
  {
  }
  // the time it takes the device to begin generating clock pulses after
  // the host initially takes the clock line low, must be no greater than 15ms 

  // where is the start bit ??

  // ready read bit 1 of data
  delayMicroseconds(CLKHALF); // must be no greater than 15ms
  set_low(_clk_pin); // falling clk, host waiting for (to set the bit 1 of data)
  delayMicroseconds(CLKFULL);
  set_high(_clk_pin); // rising clk, device will latch data, and ready next falling clk
  delayMicroseconds(CLKHALF);

  for (i=0; i < 8; i++)
  {
      if (digitalRead(_data_pin) == HIGH)
      {
        data = data | bit;
      }
      else 
      {
      }

      bit = bit << 1;
      
      delayMicroseconds(CLKHALF);
      set_low(_clk_pin);	// falling clk, host waiting for (to set the next bit)
      delayMicroseconds(CLKFULL);
      set_high(_clk_pin); // rising clk, device will latch data, and ready next falling clk
      delayMicroseconds(CLKHALF);
      
      parity = parity ^ (data & 0x01);
  }
  // we do the delay at the end of the loop, so at this point we have
  // already done the delay for the parity bit
  if (digitalRead(_data_pin) != parity)
  {
#ifdef DEBUG
    Serial.println("parity bit error");
#endif    
    ret |= 1;
  }

  // stop bit
  delayMicroseconds(CLKHALF);
  set_low(_clk_pin);	// falling clk, host waiting for (to set the stop bit)
  delayMicroseconds(CLKFULL);
  set_high(_clk_pin); // rising clk, device will latch stop bit, and ready next falling clk
  delayMicroseconds(CLKHALF);
  if (digitalRead(_data_pin) != 1)
  {
#ifdef DEBUG
    Serial.println("stop bit error");
#endif      
    ret |= 2;
  }

  // acknowledge bit
  delayMicroseconds(15); // rising clk end
  set_low(_data_pin); // host wating for device pulling data
  delayMicroseconds(5);
  set_low(_clk_pin);	 // host wating for device pulling clk
  delayMicroseconds(CLKFULL);
  set_high(_clk_pin);  // host wating for device release clk
  delayMicroseconds(5);
  set_high(_data_pin); // host wating for device release data
  /*
  delayMicroseconds(CLKHALF); // rising clk end
  set_low(_data_pin); // host wating for device pulling data
  set_low(_clk_pin);  // host wating for device pulling clk
  delayMicroseconds(CLKFULL);
  set_high(_clk_pin);  // host wating for device release clk
  delayMicroseconds(CLKHALF);
  set_high(_data_pin); // host wating for device release data
  */

  // delay 45 microseconds (to give host time to inhibit next transmission) == acknowledge bit ??
  // delayMicroseconds(45); ??

#ifdef DEBUG
  Serial.print("received ");
  Serial.println(data, HEX);
#endif

  *ptr = data;
  return ret;
}

