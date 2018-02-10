/***********************************************************
 * File:        i2c.c
 * Created on:  Nov 19, 2017
 * Author:      Rod Naugler - B00064907
 * 		        Dalhousie University
 * Description: As part of ECED4901 - Senior Design Project
 * 				For Omnitech Electronics.
 * 				This file contains all of the generic i2c code.
 * 				Individual chip code moved to separate files
 ***********************************************************/

#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

/*************************
 *  Generic I2C Routines *
 *************************/
void TWI_Start(void) {							//TWI   - Two Wire Interface
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);		//TWCR  - TWI Control Register
												//TWINT - TWI Interrupt Flag - Reset
												//TWSTA - TWI Start Condition - Assert
												//TWEN  - TWI Enable - Assert
	loop_until_bit_is_set(TWCR, TWINT);			//Wait until the TWINT flag is set, which
												//means that the start condition was successful
}
void TWI_Stop(void) {
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);		//TWINT - TWI Interrupt Flag - Reset
												//TWSTO - TWI Stop Condition - Assert
												//TWEN  - TWI Enable - Assert
	loop_until_bit_is_clear(TWCR, TWSTO);		//Wait until the TWINT flag is set, which
												//means that the stop condition was completed
}
void TWI_sendByte(uint8_t cx) {
	TWDR = cx;									//TWDR  - TWI Data Register - load the byte
	TWCR = (1<<TWINT)|(1<<TWEN);				//TWINT - TWI Interrupt Flag - Reset
												//TWEN  - TWI Enable - Assert
	loop_until_bit_is_set(TWCR, TWINT);			//Wait until the TWINT flag is set, which
												//means that the data register is now empty
}
uint8_t TWI_readByte(char sendAck) {			//Returns a byte. sendAck = 1 = TRUE
	if(sendAck){								//If we want to send the acknowledge after reception
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);	//TWINT - TWI Interrupt Flag - Reset
												//TWEN  - TWI Enable - Assert
												//TWEA  - TWI Enable Acknowledge - Assert
	} else {
		TWCR = (1<<TWINT)|(1<<TWEN);			//TWINT - TWI Interrupt Flag - Reset
												//TWEN  - TWI Enable - Assert
	}
	loop_until_bit_is_set(TWCR, TWINT);			//Wait until the TWINT flag is set, which
												//means that the data register is now full of data
	return TWDR;								//Return the data byte
}
uint8_t TWI_status(void) {						//TWSR  - TWI Status Register - Only 5 MSB are status
	return TWSR & 0xF8;							//Mask off 3LSB and return the Status Register
}

uint8_t checkDevice(uint8_t SLA){						//Check to see if there is a device at
														//the supplied address or not
	uint8_t tmp;										//A temp variable for the status
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address (write address)
	tmp = TWI_status();									//Get the TWI status byte
	TWI_Stop();											//Send the TWI Stop to release the bus
	if (tmp == 0x18){									//If the status is 0x18, the ACK was rxd
		return 1;										//So return a TRUE (present) message
	} else {											//Otherwise,
		return 0;										//Return a FALSE (absent) message
	}
}

/******************************
 *  EEPROM Specific Routines  *
 *  - NO error handling done! *
 ******************************/
void writePoll(uint8_t SLA) {					//SLA  - Slave Address
	char busy = 1;								//Status bit
	while(busy){								//Run this loop until we get a response
		TWI_Start();							//Start the TWI
		TWI_sendByte(SLA);						//Send a byte on the TWI that is the Slave Address
												//
		if(TWI_status() == 0x18){				//0x18  - SLA+W has been transmitted; ACK has been received
												//0x20  - SLA+W has been transmitted; NOT ACK has been received
			//OK
			busy = 0;							// If we recieved an ACK to the address, the device is ready
		}
	}
}
void writeByteEE(uint8_t SLA, uint8_t addr, uint8_t data) {
	TWI_Start();
	TWI_sendByte(SLA);
	TWI_sendByte(addr);
	TWI_sendByte(data);
	TWI_Stop();
	writePoll(SLA);
}
uint8_t readByteEE(uint8_t SLA, uint8_t addr) {
	uint8_t tmp;
	TWI_Start();
	TWI_sendByte(SLA);
	TWI_sendByte(addr);
	TWI_Start();
	TWI_sendByte(SLA | 0x01);
	tmp = TWI_readByte(0);
	TWI_Stop();
	return tmp;
}
//You can extend these to have error handling - see http://www.embedds.com/programming-avr-i2c-interface/
//for example

/************************
 * MAX5217 DAC Routines *
 ************************/
void setDACoutput(uint8_t SLA, uint16_t value) {
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address
	TWI_sendByte(0x01);									//Send the CODE_LOAD command - immediately
														//set the following 16 bit value on the DAC output
	TWI_sendByte(value>>8);								//Send the high-byte
	TWI_sendByte(value&0xff);							//Send the low-byte
	TWI_Stop();											//Send the TWI Stop
}
uint16_t readDAC(uint8_t SLA, uint8_t reg){
	uint16_t tmp;										//The 16-bit value to return
	uint8_t tmph;										//High byte
	uint8_t tmpl;										//Low byte
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address (write address)
	TWI_sendByte(reg);									//Send the address to read from
	TWI_Start();										//Send a repeated start
	TWI_sendByte(SLA+1);								//Send the slave address+1 (read address)
	tmph = TWI_readByte(1);								//Read data from TWI (high byte) ACK
	tmpl = TWI_readByte(1);								//Read data from TWI (low byte) ACK
	TWI_Stop();											//Send the TWI Stop
	tmp=tmph << 8;										//Move the high byte into the high byte position
	tmp=tmp+tmpl;										//Add the low byte
	return tmp;											//Return the 16-bit value

}

void resetDAC(uint8_t SLA) {
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address (write address)
	TWI_sendByte(0x08);									//Send the User Config Command
	TWI_sendByte(0x00);									//Send the 'Don't Care' byte
	TWI_sendByte(0x00);									//Send the config byte
														//xxCCAAPP CC - Clear Value Mode, AA - AUX mode, PP - Powerdown Mode
														//cc - Default, cC - Zero, Cc - Mid, CC - Full
														//aa - Disable, aA - LDAC, Aa - Clear, AA - Disable
														//pp - DAC, pP - High Z , Pp - 100k, PP - 1k
	TWI_Stop();											//Send the TWI Stop
}

/***********************************
 * LTC2946 Energy Monitor Routines *
 ***********************************/
/*void setDACoutput(uint8_t SLA, uint16_t value) {
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address
	TWI_sendByte(0x01);									//Send the CODE_LOAD command - immediately
														//set the following 16 bit value on the DAC output
	TWI_sendByte(value>>8);								//Send the high-byte
	TWI_sendByte(value&0xff);							//Send the low-byte
	TWI_Stop();											//Send the TWI Stop
}
uint16_t readDAC(uint8_t SLA, uint8_t reg){
	uint16_t tmp;										//The 16-bit value to return
	uint8_t tmph;										//High byte
	uint8_t tmpl;										//Low byte
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address (write address)
	TWI_sendByte(reg);									//Send the address to read from
	TWI_Start();										//Send a repeated start
	TWI_sendByte(SLA+1);								//Send the slave address+1 (read address)
	tmph = TWI_readByte(1);								//Read data from TWI (high byte) ACK
	tmpl = TWI_readByte(1);								//Read data from TWI (low byte) ACK
	TWI_Stop();											//Send the TWI Stop
	tmp=tmph << 8;										//Move the high byte into the high byte position
	tmp=tmp+tmpl;										//Add the low byte
	return tmp;											//Return the 16-bit value

}
*/

uint8_t readEM_Status(uint8_t SLA) {
	uint8_t tmp;										//The 8-bit value to return
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address (write address)
	TWI_sendByte(0x03);									//Send the address to read from
	TWI_Start();										//Send a repeated start
	TWI_sendByte(SLA+1);								//Send the slave address+1 (read address)
	tmp = TWI_readByte(0);
	TWI_Stop();											//Send the TWI Stop
	return tmp;											//Return the 16-bit value
}
uint16_t readEM_CTRL(uint8_t SLA) {
	uint16_t tmp;										//The 16-bit value to return
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address (write address)
	TWI_sendByte(0x00);									//Send the address to read from
	TWI_Start();										//Send a repeated start
	TWI_sendByte(SLA+1);								//Send the slave address+1 (read address)
	tmp = (TWI_readByte(1)<<8);							//Read data from TWI (high byte) ACK
	tmp += TWI_readByte(0);								//Read data from TWI (low byte) NACK
	TWI_Stop();											//Send the TWI Stop
	return tmp;											//Return the 16-bit value
}

uint32_t readEM_Time(uint8_t SLA) {
	uint32_t tmp=0;										//The 32-bit value to return
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address (write address)
	TWI_sendByte(0x34);									//Send the address to read from
	TWI_Start();										//Send a repeated start
	TWI_sendByte(SLA+1);								//Send the slave address+1 (read address)
	tmp = TWI_readByte(1);								//Read data from TWI (high byte 3) ACK
	tmp = (tmp<<8) + TWI_readByte(1);					//Read data from TWI (high byte 2) ACK
	tmp = (tmp<<8) + TWI_readByte(1);					//Read data from TWI (high byte 1) ACK
	tmp = (tmp<<8) + TWI_readByte(0);					//Read data from TWI (low byte) NACK
	TWI_Stop();											//Send the TWI Stop
	return tmp;											//Return the 32-bit value
}
uint16_t readEM_VOLT(uint8_t SLA) {
	uint16_t tmp;										//The 16-bit value to return
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address (write address)
	TWI_sendByte(0x1E);									//Send the address to read from
	TWI_Start();										//Send a repeated start
	TWI_sendByte(SLA+1);								//Send the slave address+1 (read address)
	tmp = (TWI_readByte(1)<<4);							//Read data from TWI (high byte) ACK
	tmp += TWI_readByte(0)>>4;								//Read data from TWI (low byte) NACK
	TWI_Stop();											//Send the TWI Stop
	return tmp;											//Return the 16-bit value
}
uint16_t readEM_CURRENT(uint8_t SLA) {
	uint16_t tmp;										//The 16-bit value to return
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address (write address)
	TWI_sendByte(0x14);									//Send the address to read from
	TWI_Start();										//Send a repeated start
	TWI_sendByte(SLA+1);								//Send the slave address+1 (read address)
	tmp = (TWI_readByte(1)<<4);							//Read data from TWI (high byte) ACK
	tmp += TWI_readByte(0)>>4;								//Read data from TWI (low byte) NACK
	TWI_Stop();											//Send the TWI Stop
	return tmp;											//Return the 16-bit value
}
uint32_t readEM_POWER(uint8_t SLA) {
	uint32_t tmp=0;										//The 32-bit value to return
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address (write address)
	TWI_sendByte(0x05);									//Send the address to read from
	TWI_Start();										//Send a repeated start
	TWI_sendByte(SLA+1);								//Send the slave address+1 (read address)
	tmp = TWI_readByte(1);						        //Read data from TWI (high byte2) ACK
	tmp = (tmp<<8) + TWI_readByte(1);						//Read data from TWI (high byte1) ACK
	tmp = (tmp<<8) + TWI_readByte(0);						//Read data from TWI (low byte) NACK
	TWI_Stop();											//Send the TWI Stop
	return tmp;											//Return the 32-bit value
}
uint32_t readEM_ENERGY(uint8_t SLA) {
	uint32_t tmp;										//The 16-bit value to return
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address (write address)
	TWI_sendByte(0x3C);									//Send the address to read from
	TWI_Start();										//Send a repeated start
	TWI_sendByte(SLA+1);								//Send the slave address+1 (read address)
	tmp = (TWI_readByte(1)*16777216);					//Read data from TWI (high byte 3) ACK
	tmp += (TWI_readByte(1)*65536);						//Read data from TWI (high byte2) ACK
	tmp += (TWI_readByte(1)<<8);						//Read data from TWI (high byte1) ACK
	tmp += TWI_readByte(0);								//Read data from TWI (low byte) NACK
	TWI_Stop();											//Send the TWI Stop
	return tmp;											//Return the 16-bit value
}
void sendEM_RESET_ACC(uint8_t SLA){
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address
	TWI_sendByte(0x01);									//Send the CTRLB Register Address
	TWI_sendByte(0x02);									//Send the reset accumulator command
	TWI_Stop();
	TWI_Start();										//Send the TWI Start
	TWI_sendByte(SLA);									//Send the slave address
	TWI_sendByte(0x01);									//Send the CTRLB Register Address
	TWI_sendByte(0x00);									//Send the reset accumulator command

}
