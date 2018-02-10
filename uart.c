/***********************************************************
 * File:        uart.c
 * Created on:  Nov 17, 2017
 * Author:      Rod Naugler - B00064907
 * 		        Dalhousie University
 * Description: As part of ECED4901 - Senior Design Project
 * 				For Omnitech Electronics.
 * 				These functions setup and control
 * 				UART Communications for ATMega series MCUs
 * 				Based on code provided in ENGI2203
 ***********************************************************/

#include <stdio.h>
#include <avr/io.h>

int uart_putchar(char c, FILE *stream);
int uart_getchar(FILE *stream);
FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE mystdin = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

int uart_putchar(char c, FILE *stream) {
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    return 0;
}
int uart_getchar(FILE *stream) {
    /* Wait until data exists. */
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}


void init_uart(void) {								//Function to initialize the UART
	UCSR0B = 0;
	UCSR0B |= 1<<TXEN0;								//Receiver Enable 0
	UCSR0B |= 1<<RXEN0;								//Transmitter Enable 0
	UCSR0B |= 1<<RXCIE0;							//Receive Interupt Enable

    UBRR0 = 8;		//USART Baud Rate
					//  BAUD    16MHz   14.7456MHz
					//  9600   103      95
					// 14400    68      63
					// 57600    16      15
					//115200     8       7

    stdout = &mystdout;
    stdin = &mystdin;
}
