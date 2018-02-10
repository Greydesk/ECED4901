/***********************************************************
 * File:        Queues.c
 * Created on:  Feb 6, 2017
 * Author:      Rod Naugler
 *              B00064907
 * Description: As part of ECED4901 - Senior Design Project
 * 				For Omnitech Electronics.
 * 				INPUT queue is a circular queue holding
 * 				    inputs from UART
 *              COMMAND queue is a linear queue holding
 *                  characters from INPUT to be interpreted
 *                  as a command
 *              enqueue functions add characters to the end
 *                  of the queue
 *              dequeue functions removed characters from
 *                  the front of the queue
 ***********************************************************/

/***********************************************************
 * Include the header file for queues
 ***********************************************************/
#include "queues.h"

/***********************************************************
 * Create an array of input chars for processing as a
 * circular queue
 ***********************************************************/
char INPUT[ INPUT_QUEUE_SIZE ];

/***********************************************************
 * These three integers control the circular queue
 * The head is the location of the next character to be sent
 * The tail is the location of the end of the queue
 * The size is the number of characters stored in the array
 * As items are removed from the head, head is incremented
 *   (with wrap around) and count is decremented
 * As items are added to the tail, tail is incremented
 *   (with wrap around) and count is incremented
 * If count reaches Output_Queue_Size, the queue is full.
 *   Head and tail should be adjacent
 * If count reaches 0, the queue is empty.
 *   Head should = tail.
 ***********************************************************/
int input_head = 0, input_tail = 0, input_size = 0;

/***********************************************************
 * Function to add a UART RXD character into a circular
 * queue. Takes a single character as input.
 ***********************************************************/
void input_enqueue(char character){
    //If the queue is not full
    if (input_size < INPUT_QUEUE_SIZE){
        //Disable CPU interrupts
        cli();
        //Put the character into the array at input_tail position
        INPUT[input_tail]=character;
        //Move the input_tail to the next array location
        input_tail++;
        /***************************************************
         * If tail is at the end of the array,
         * wrap around to the beginning
         ***************************************************/
        if(input_tail >= INPUT_QUEUE_SIZE){
            input_tail = 0;
        }
        //Increment the input queue size
        input_size++;

        //Enable CPU interrupts
        sei();

    }
    return;
}
/***********************************************************
 * Function to remove an item from the input queue. Does not
 * return any character, only removes the current head item
 * from the queue.
 ***********************************************************/
void input_dequeue() {
    //Disable CPU interrupts
    cli();

    //Move head to the next array location
    input_head++;

    /*******************************************************
     * If head is at the end of the array,
     * wrap around to the beginning
     *******************************************************/
    if (input_head >= INPUT_QUEUE_SIZE){
        input_head = 0;
    }

    //Decrement the uart queue size
    input_size--;

    //Enable CPU interrupts
    sei();
}
/***********************************************************
 * Create an array of characters to hold the command.
 ***********************************************************/
char COMMAND[ COMMAND_SIZE ];

/***********************************************************
 * Instead of a circular queue, this will be an array that
 * can be used to hold the command, rather than using a char*
 ***********************************************************/
int command_head = 0, command_size = 0;

