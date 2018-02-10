/*
 * i2c.h
 *
 *  Created on: Nov 19, 2017
 *      Author: statik
 */

#ifndef I2C_H_
#define I2C_H_

//Generic TWI functions
extern void TWI_Start(void);
extern void TWI_Stop(void);
extern void TWI_sendByte(uint8_t cx);
extern uint8_t TWI_readByte(char sendAck);
extern uint8_t TWI_status(void);
extern uint8_t checkDevice(uint8_t SLA);

//EEPROM functions
extern void writePoll(uint8_t SLA);
extern void writeByteEE(uint8_t SLA, uint8_t addr, uint8_t data);
extern uint8_t readByteEE(uint8_t SLA, uint8_t addr);

//DAC functions
extern void setDACoutput(uint8_t SLA, uint16_t value);
extern void resetDAC(uint8_t SLA);
extern uint16_t readDAC(uint8_t SLA, uint8_t reg);

//Energy Monitor Functions
extern uint16_t readEM_CTRL(uint8_t SLA);
extern uint8_t readEM_Status(uint8_t SLA);
extern uint32_t readEM_Time(uint8_t SLA);
extern uint16_t readEM_VOLT(uint8_t SLA);
extern uint16_t readEM_CURRENT(uint8_t SLA);
extern uint32_t readEM_POWER(uint8_t SLA);
extern void sendEM_RESET_ACC(uint8_t SLA);
extern uint32_t readEM_ENERGY(uint8_t SLA);
#endif /* I2C_H_ */
