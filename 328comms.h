/*
 * 328comms.h
 *
 *  Created on: Nov 17, 2017
 *      Author: statik
 */

#ifndef COMMS_H_
#define COMMS_H_

#ifndef F_CPU
#define F_CPU 16000000
#endif // F_CPU

#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

#define EM_ADDR      0xDE		//LTC2946 Energy monitor address
#define DAC_ADDR     0x38		//MAX5217 DAC address
#define EEPROM_ADDR  0xA0		//24LC08B EEPROM address
#define ADC_ADDR     0x14		//LTC2451 ADC address

#define ESC 27					//VT100 Escape code

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>	//Include the AVR Interrupt Handling  Library
#include <util/delay.h>		//Include the delay library

#include "uart.h"			//Include the UART communication functions

#include "i2c.h"			//Include the I2C communication functions

#include "queues.h"			//Include the input queues functions

#endif /* COMMS_H_ */
