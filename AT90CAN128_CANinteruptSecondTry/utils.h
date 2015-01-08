/*****************************************************************************
*
* CAN Test-APP
*
*
* Compiler : avr-gcc 4.3.0 / avr-libc 1.6.2 / AVR Studio 4.14
* size     : 3,28KB
* by       : Thomas Fuchs, Wolfsburg, Germany
*            linux@cc-robotics.de
*
* License  : Copyright (c) 2009 Thomas Fuchs
*
* Tested with AT90CAN128
****************************************************************************/
#include <util/delay.h>
#include <util/atomic.h>

// ----------------------------------------------------------------------------
#ifndef	TRUE
	#define	TRUE	(1==1)
#elif !TRUE
	#error	fehlerhafte Definition fuer TRUE
#endif

#ifndef FALSE
	#define	FALSE	(1!=1)
#elif FALSE
	#error	fehlerhafte Definition fuer FALSE
#endif

#ifndef NULL
	#define NULL ((void*)0)		//!< Nullzeiger
#endif

// ----------------------------------------------------------------------------
/**
 *  conversion
 */

#define	DEGREE_TO_RAD(x)	((x * M_PI) / 180)
// ----------------------------------------------------------------------------
/**
 *  BYTE order
 */

#define	LOW_BYTE(x)		((uint8_t) (x & 0xff))
#define	HIGH_BYTE(x)	((uint8_t) (x >> 8))
#define LOW_WORD(x)		((uint16_t) (x & 0xffff))
#define HIGH_WORD(x)    ((uint16_t) (x >> 16))
// ----------------------------------------------------------------------------
/**
 *  typechanging
 */
typedef struct {
	uint8_t b4;		// lsb
	uint8_t b3;
	uint8_t b2;
	uint8_t b1;		// msb
} tLongToByte;

// ----------------------------------------------------------------------------
#define	PORT(x)			_port2(x)
#define	DDR(x)			_ddr2(x)
#define	PIN(x)			_pin2(x)
#define	REG(x)			_reg(x)
#define	PIN_NUM(x)		_pin_num(x)

#define	RESET(x)		RESET2(x)
#define	SET(x)			SET2(x)
#define	TOGGLE(x)		TOGGLE2(x)
#define	SET_OUTPUT(x)	SET_OUTPUT2(x)
#define	SET_INPUT(x)	SET_INPUT2(x)
#define	SET_PULLUP(x)	SET2(x)
#define	IS_SET(x)		IS_SET2(x)

#define	SET_INPUT_WITH_PULLUP(x)	SET_INPUT_WITH_PULLUP2(x)

#define	_port2(x)	PORT ## x
#define	_ddr2(x)	DDR ## x
#define	_pin2(x)	PIN ## x

#define	_reg(x,y)		x
#define	_pin_num(x,y)	y

#define	RESET2(x,y)		PORT(x) &= ~(1<<y)
#define	SET2(x,y)		PORT(x) |= (1<<y)
#define	TOGGLE2(x,y)	PORT(x) ^= (1<<y)

#define	SET_OUTPUT2(x,y)	DDR(x) |= (1<<y)
#define	SET_INPUT2(x,y)		DDR(x) &= ~(1<<y)
#define	SET_INPUT_WITH_PULLUP2(x,y)	SET_INPUT2(x,y);SET2(x,y)

#define	IS_SET2(x,y)	((PIN(x) & (1<<y)) != 0)
// ----------------------------------------------------------------------------
/**
 *  direct bit test
 */
#define	_bit_is_set(pin, bit)	(pin & (1<<bit))
#define	_bit_is_clear(pin, bit)	(!(pin & (1<<bit)))

#define	STRING(x)	_STRING(x)
#define	_STRING(x)	#x
