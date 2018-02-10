/***********************************************************
 * File:        328comms.c
 * Created on:  Nov 17, 2017
 * Author:      Rod Naugler - B00064907
 * 		        Dalhousie University
 * Description: As part of ECED4901 - Senior Design Project
 * 				For Omnitech Electronics.
 * 				This is the main program for the project.
 * 				Presents a menu and moves through different
 * 				states depending on choices
 ***********************************************************/

#include "328comms.h"			//Include the core definitions

/*
 * Create a mode variable to track which mode we are in and
 * start in constant current mode
 */

enum MODE mode=CURRENT;

/***********************************************************
 * The following flags are used to activate different routines
 * if the devices are present or not
 ***********************************************************/
uint8_t adcPresent = 0;			//Flag for ADC present
uint8_t dacPresent = 0;			//Flag for DAC present
uint8_t eepromPresent = 0;		//Flag for EEPROM present
uint8_t energyPresent = 0;		//Flag for Energy monitor present

/***********************************************************
 * Variables for reading the Energy Chip Time
 ***********************************************************/
uint32_t measured_time=0;
unsigned int days=0, hours=0, minutes=0, seconds=0, milliseconds=0;

/***********************************************************
 * printDeviceStatus takes the device address and a pointer
 * to the device present flag. It sets the device flag
 * based on the response of the checkDevice function
 ***********************************************************/
void printDeviceStatus(uint8_t SLA, uint8_t * deviceFlag){
	if (checkDevice(SLA)==1){
		printf("Present");
		*deviceFlag=1;
	} else {
		printf("Absent");
		*deviceFlag=0;
	}
}

void printNiceTime(uint8_t SLA){
	/*******************************************************
	 * Convert the timer register to human readable format
	 *            ticks    secs  mins  hour
	 * secs          60
	 * mins        3600      60
	 * hours     216000    3600    60
	 * days     5184000   86400  1440    24
	 * 60 ticks (milliseconds) per second
	 *******************************************************/
	measured_time = readEM_Time(EM_ADDR);
	days = measured_time/5184000;
	hours = (measured_time/216000)-(days*24);
	minutes = (measured_time / 3600)-(hours*60)-(days*1440);
	seconds = (measured_time / 60)-(minutes*60)-(hours*3600)-(days*86400);
	milliseconds = measured_time-(seconds*60)-(minutes*3600)-(hours*216000)-(days*5184000);
	printf("%03d %02d:%02d:%02d.%02d", days, hours, minutes, seconds, milliseconds);
}

/***********************************************************
 * UART RX ISR Function
 * Function is executed every time a character is received
 * by the UART. It merely enqueues the character into the
 * input queue and resets the flag
 ***********************************************************/
ISR(USART_RX_vect){

	input_enqueue(UDR0);
}
/***********************************************************
 * Main Function
 **************************************************************/
int main(void) {

	init_uart();				//Initialize the USART

	sei();						//Turn on Global Interrupts

	/**********************************************************
	 * i2c Setup
     * TWBR  - TWI Bit Rate Register
	 *          16MHz     14.7456MHz
	 * 50kHz     152       139
	 **********************************************************/
	TWBR = 152;					//50 kHz I2C frequency (slower than normal)
	TWCR = 1<<TWEN;				//Enable I2C
	TWSR = 0;

	//Reset the Energy Chip
	sendEM_RESET_ACC(EM_ADDR);
	//Reset the DAC
	resetDAC(DAC_ADDR);

	uint16_t measured_voltage=0, measured_current=0;
	uint32_t measured_power=0, measured_energy=0;
	float voltage_conversion = 0.024944;
	float current_conversion = 0.07125;
	float power_conversion = voltage_conversion*current_conversion/1000;
	float DAC_conversion = .004543;
	float DAC_offset = 0;
	float calculated_voltage=0, calculated_current=0, desired_current=0, calculated_DAC=0;
	float calculated_power=0, calculated_energy=0, desired_power=0;
	calculated_DAC=desired_current/DAC_conversion+DAC_offset;

	uint16_t input;

	//Print a nice screen
	printf("%c[2J",ESC);		//Clear the Screen
	printf("%c[?25l",ESC);		//Hide the Cursor
	printf("%c[1;1H",ESC);		//Move to top left corner of screen
	printf("                      --== Omnitech Electronics Inc. ==--                       ");
	printf("%c[15;1H",ESC);		//Move down to menu position
	printf("+------------------------------------------------------------------------------+");
	printf("|                                Menu                                          |");
	printf("+------------------------------------------------------------------------------+");
	printf("| R/r - Reset Accumulator          | +/- Change Set Point by 10mA/0.1W         |");
	printf("| P/p - Set Constant Power Mode    | C/c - Set Constant Current Mode           |");

	//Is the Energy Monitor Present?
	printf("%c[3;3HEnergy Monitor:%c[3;20H",ESC,ESC);
	printDeviceStatus(EM_ADDR,&energyPresent);

	//Is the DAC Present?
	printf("%c[3;40HDAC:%c[3;60H",ESC,ESC);
	printDeviceStatus(DAC_ADDR,&dacPresent);

	//Is the EEPROM Present?
	printf("%c[4;3HEEPROM:%c[4;20H",ESC,ESC);
	printDeviceStatus(EEPROM_ADDR,&eepromPresent);

	//Is the ADC Present?
	printf("%c[4;40HADC:%c[4;60H",ESC,ESC);
	printDeviceStatus(ADC_ADDR,&adcPresent);

	while (1) {

		measured_voltage=readEM_VOLT(EM_ADDR);
		calculated_voltage=(float)measured_voltage*voltage_conversion;
		measured_current=readEM_CURRENT(EM_ADDR);
		calculated_current=(float)measured_current*current_conversion;
		measured_power=readEM_POWER(EM_ADDR);
		calculated_power=(double)measured_power*power_conversion;
		measured_energy=readEM_ENERGY(EM_ADDR);
		calculated_energy=measured_energy*power_conversion/60;


		printf("%c[6;1H  Run Time: %c[6;20H",ESC, ESC);
		printNiceTime(EM_ADDR);

		printf("%c[6;40HEnergy:%c[6;60H%f W/s",ESC,ESC,calculated_energy);
		printf("%c[7;1H  Voltage:%c[7;20H%7.03f V%c[7;40HPower:%c[7;60H%6.03f  W",ESC,ESC,calculated_voltage,ESC,ESC,calculated_power);
		if (mode == POWER){
			printf("%c[8;1H  Power Set:  %c[8;20H%7.03f W %c[8;40HCurrent Measured:%c[8;60H%7.03f mA",ESC,ESC,desired_power,ESC,ESC,calculated_current);
		}else {
			printf("%c[8;1H  Current Set:%c[8;20H%7.03f mA%c[8;40HCurrent Measured:%c[8;60H%7.03f mA",ESC,ESC,desired_current,ESC,ESC,calculated_current);
		}
		printf("%c[10;1H",ESC);
//		printf("  EM Status 0x3E is %x\n", readEM_Status(EM_ADDR));
//		printf("  EM CTRL value is %04x\n", readEM_CTRL(EM_ADDR));

		printf("  EM DELTA SENSE VALUE is %04x\n", measured_current);
		printf("  Measured Current: %08ld\n  Measured Voltage:  %08ld\n  Measured Power: %lx  -  %f\n", (long)measured_current, (long)measured_voltage, measured_power, measured_power);
		//While there are items in the input queue
		if(input_size>0){
			while(input_size>0){
				//Get the current head item from the queue
				input = INPUT[input_head];
				//dequeue the head item
				input_dequeue();
				if (input != 0x1b){
					switch (input){
					case 'r':
						sendEM_RESET_ACC(EM_ADDR);
						break;
					case 'R':
						sendEM_RESET_ACC(EM_ADDR);
						break;
						//0x2b = +
					case 0x2b:
						if (mode == CURRENT){
							if(desired_current <=290){
								desired_current += 10;
							}
						} else {
							if(desired_power <=9.9){
								desired_power += 0.1;
							}
						}
						break;
						//0x2d = -
					case 0x2d:
						if (mode == CURRENT){
							if(desired_current >10){
								desired_current -= 10;
							} else {
								desired_current = 0;
							}
						} else {
							if(desired_power > 0.1){
								desired_power -= 0.1;
								if(desired_power<0.1){
									desired_power = 0;
									desired_current=0;
									calculated_DAC=0;
								}
							} else {
								desired_power = 0;
								desired_current=0;
								calculated_DAC=0;
							}
						}
						break;
					case 'C':
						mode = CURRENT;
						break;
					case 'c':
						mode = CURRENT;
						break;
					case 'P':
						mode = POWER;
						break;
					case 'p':
						mode = POWER;
						break;
					default:
						printf("%c[20;1H Unrecognized Command",ESC);
					}
					if (mode == CURRENT){
						calculated_DAC=desired_current/DAC_conversion+DAC_offset;
					} else {
						if (desired_power == 0){
							desired_current = 0;
							calculated_DAC = 0;
						} else {
							calculated_DAC = ((desired_power*1000)/calculated_voltage)/DAC_conversion;
						}
					}
				}
			}
		} else {
			if (mode == POWER){
				printf("%c[21;1H %f, %f %f %x",ESC,desired_power, desired_current,calculated_current, calculated_DAC);
				if(desired_power <= 0) {
					desired_power = 0;
					desired_current = 0;
					calculated_DAC = 0;
				} else {
					desired_current = (desired_power*1000)/calculated_voltage;
				}
			}
			if(desired_current == 0) {
				calculated_DAC = 0;
			} else {
				if(desired_current > calculated_current){
					if (desired_current-calculated_current>0.1){
						calculated_DAC += ((desired_current-calculated_current)*10);
					}
				} else {
					if (desired_current < calculated_current){
						if (calculated_current-desired_current>0.1) {
							calculated_DAC -= ((calculated_current-desired_current)*10);
						}
					}
				}
			}
		}
		setDACoutput(DAC_ADDR, calculated_DAC);
	}


}
