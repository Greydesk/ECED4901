/***********************************************************
 * File:        uart.h
 * Created on:  Nov 17, 2017
 * Author:      Rod Naugler - B00064907
 * 		        Dalhousie University
 * Description: As part of ECED4901 - Senior Design Project
 * 				For Omnitech Electronics.
 * 				This is the header for the UART Code that
 * 				supports UART Communications for
 * 				ATMega series MCUs
 ***********************************************************/

#ifndef UART_H_
#define UART_H_

extern int uart_putchar(char c, FILE *stream);		//Allow external access to the uart_putchar function
extern int uart_getchar(FILE *stream);				//Allow external access to the uart_getchar function
extern void init_uart(void);						//Allow external access to init_uart function

#endif /* UART_H_ */
