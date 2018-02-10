/***********************************************************
 * File:        Queues.h
 * Created on:  Feb 6, 2017
 * Author:      Rod Naugler - B00064907
 * 		        Dalhousie University
 * Description: As part of ECED4901 - Senior Design Project
 * 				For Omnitech Electronics.
 * 				This is the header for the Queues Code that
 * 				supports UART Input and Output queues
 * 				ATMega series MCUs
 ***********************************************************/

#ifndef QUEUES_H_
#define QUEUES_H_

#include <avr/interrupt.h>	//Include the AVR Interrupt Handling  Library

#define INPUT_QUEUE_SIZE        32
#define COMMAND_SIZE            16
#define ESCAPE					27

//Allow external access to the two main queues
extern void input_enqueue(char character);
extern void input_dequeue();

//Allow external access to the COMMAND queue data
extern char COMMAND[];
extern int command_head;
extern int command_size;
extern int max_command_size;

//Allow external access to the INPUT queue data
extern char INPUT[];
extern int input_head;
extern int input_tail;
extern int input_size;

//The system mode variable.
enum MODE {
    CURRENT,				//Constant Current Mode
    POWER					//Constant Power Mode
};

extern enum MODE mode;

//Allow external access to the UART_TX busy flag
extern int UART_TX;

#endif /* QUEUES_H_ */
