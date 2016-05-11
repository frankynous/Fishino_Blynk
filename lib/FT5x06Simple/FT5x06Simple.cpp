/************************************************************************
This is a library to handling capacitive multitouch sensors using FT5x06.
Originally written to work with ER-TFTM070-5 (LCD module) from EastRising.

Written by Helge Langehaug, February 2014
Added Due Support Johan Lieshout

BSD license, all text above must be included in any redistribution
*************************************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <FT5x06Simple.h>

#define MKUINT16(a, b) ( ((uint16_t)a) << 8 | (b) )

volatile bool touchInt;

// constructor
// parameter is arduino's interrupt pin
FT5x06::FT5x06(uint8_t CTP_INT)
{
	_ctpInt = CTP_INT;

	x = y = 0;
}

void touch_interrupt()
{
	touchInt = true;
}

bool FT5x06::touched()
{
	if (!touchInt)
		return false;
	touchInt = false;

	// read first 6 registers.... enough to get first touch point and other data
	Wire.requestFrom(FT5206_I2C_ADDRESS, 7);

	// discard DEVIDE_MODE register
	Wire.read();

	// discard GEST_ID register
	Wire.read();

	// read needed registers

	// TD_STATUS
	uint8_t touchStatus = Wire.read();

	// TOUCH1_XH
	uint8_t xh = Wire.read();

	// TOUCH1_XL
	uint8_t xl = Wire.read();

	// TOUCH1_YH
	uint8_t yh = Wire.read();

	// TOUCH1_YL
	uint8_t yl = Wire.read();

	// do nothing if no touch points
	if (!(touchStatus & 0x0f))
		return false;

	// get type of event
	uint8_t event = xh >> 6;

	// discard any non-put event
	if (event != 0)
		return false;

	// signal event and store its coordinates
	x = MKUINT16(xh & 0x0f, xl);
	y = MKUINT16(yh & 0x0f, yl);

	return true;
}

// get last touch position
void FT5x06::getTouchPosition(uint16_t &_x, uint16_t &_y)
{
	_x = x;
	_y = y;
}

void FT5x06::init(void)
{
	// Interrupt
	pinMode(_ctpInt, INPUT);

#if defined(__SAM3X8E__)
	attachInterrupt(_ctpInt,touch_interrupt,FALLING);
#endif
#if defined(__AVR__)
	if (_ctpInt == 2)
		attachInterrupt(0 ,touch_interrupt, FALLING);
#endif

	x = y = 0;

	Wire.beginTransmission(FT5206_I2C_ADDRESS);
	Wire.write(FT5206_DEVICE_MODE);
	Wire.write(0);
	Wire.endTransmission(FT5206_I2C_ADDRESS);
}
