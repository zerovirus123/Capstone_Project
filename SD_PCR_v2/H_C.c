/*
 * H_C_Test.c
 *
 * Created: 3/7/2016 3:59:38 PM
 *  Author: martijus
 */ 


//Defines:
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// Libraries:
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "Timer.h"
#include "ADC.h"
#include "Battery.h"

volatile unsigned int count_ms = 0;
volatile unsigned int timerFlag = 0;
volatile unsigned int printFlag = 0;
void Counter_init (void);

ISR(TIMER1_COMPA_vect){
	count_ms ++;
	timerFlag = 1;
	printFlag ++;
}

// Global Variables:
uint8_t input;

volatile uint8_t denTime;
volatile uint8_t denTemp;
volatile uint8_t annTime;
volatile uint8_t annTemp;
volatile uint8_t eloTime;
volatile uint8_t eloTemp;
volatile uint8_t preheatTemp;
volatile uint8_t preheatTime; 

char passData[25];

// Testing:
#define LED_ON		(PORTD	|= (1<< PD6))	// Toggle Teensy LED on.
#define LED_OFF		(PORTD	&= ~(1<<PD6))	// Toggle Teensy LED off.

//// Heating:
//#define Heat_ON		(PORTB	|= (1<< PB2))	// Toggle H-bridge channel 1 on.
//#define Heat_OFF	(OCR0A = 255)			// Toggle H-bridge channel 1 off.

// Fan:
#define Fan_ON		(PORTB	|= (1<< PB3))	// Toggle H-bridge channel 2 on.
#define Fan_OFF		(PORTB	&= ~(1<<PB3))	// Toggle H-bridge channel 2 off.

#define NORM_PWM	160 - (targetTemp - readTemp)//128 - (targetTemp - readTemp) // typical PWM heating for temps below 85
#define HIGH_PWM	128 - (targetTemp - readTemp)//96 - (targetTemp - readTemp) // PWM heating at high temperatures

#define NORM_IDLE	192 // normal idle PWM output to heaters (idle is when temp is in range of target temp)
#define HIGH_IDLE	120//96 // high temperature idle PWM output to heaters

#define HEAT_OFF	255 // 0% duty cycle, turns heater off
#define HEAT_SOFF	240 // sort of off heating

//Function declarations:
//void init(void);
void port_init(void);
void init(void);
void set_PCR(void);

// Global Variables:
uint8_t input;
char passData[25];

void send_serial (uint16_t);
void send_strB(char *s);
double convertTemp(double);






void h_c_init(void){
	
	DDRB |= ((1<<PB1)|(1<<PB2)|(1<<PB3)) ; // Sets the data direction for Port B PB1 (Relay Control), PB2 is Heating PB3 is fan as an output.
	DDRF |= (0<<PF0) | (0<<PF1); // Sets the data direction for port F, PF0 (ADC in), PF1 (Low Battery Signal) as an input.
	DDRD |= (1<<6); // Sets the data direction for port D , PD6 (Teensy LED) as an output.
}

void holdAtTemp(uint16_t targetTemp, uint16_t targetTime){
	printStr("In hold at temp \n"); //send 
	
	double readTemp;
	double temp = 0;
	double avgTemp = 0.0;		
	int numTemps = 0;
	
	count_ms = 0;
	printFlag = 0;
		
	while(count_ms < 2*targetTime){//count increments at half seconds
			
		readTemp = convertADC(adc_read());
		
		_delay_ms(1);// don't forget about the delay in the adc_read function!
		
		// if within range, below target temp power heater
		if(readTemp < (targetTemp-0.5)){
			//OCR0A = 255 - (255/95)*targetTemp;
			OCR0A = NORM_PWM;
			PORTB = Fan_OFF;
			
			if (targetTemp > 85){
				OCR0A = HIGH_PWM;
			}
		}			
		// within range but above temp, activate fan
		else if (readTemp > targetTemp+0.5){
			OCR0A = HEAT_OFF;
			PORTB = Fan_ON;
			
			if (readTemp > 90){
				OCR0A = HEAT_SOFF;
			}
		}
		else{ // otherwise, in range, turn off fan and apply idle heating
			PORTB = Fan_OFF;
			OCR0A = NORM_IDLE;
			
			if (targetTemp > 85){
				OCR0A = HIGH_IDLE;
			}
			
			if (targetTemp < 60){
				OCR0A = HEAT_OFF;
			}
		}
		
		
		/*	
		if (readTemp < targetTemp){
			//OCR0A = (128 - (targetTemp-45));	// normalized to PWM @ 82 at 55C and increase duty cycle by 2 PWM values for each degree
			OCR0A = 128 - (targetTemp - readTemp);
			PORTB = Fan_OFF;					// turn off fan
			//PORTD = LED_OFF;					// turn off led
		}
		//else{
			//OCR0A = 255;		// set heater to 0% duty cycle
			//PORTB = Fan_OFF;	// power off the fan
			////PORTD = LED_ON;		// turn on the led
		//}
		
		if (readTemp > (targetTemp + 1)){
			OCR0A = 255;		// heater PWM duty cycle set to 0%
			PORTB = Fan_ON;		// turn on the fan
			//PORTD = LED_OFF;	// turn off led
			if (timerFlag==1){
				PORTB = Fan_OFF;
			}			
		}
		//else{
			//PORTB = Fan_OFF;
		//}
		*/
		
		// handles large cool-down from denaturing to annealing BROKEN!!
		//while(targetTemp < (readTemp - 15)){
			//while(fanFlag < 10){
				//PORTB = Fan_ON;
			//}
			//
			//fanFlag = 0;
			//
			//while(fanFlag < 4){
				//PORTB = Fan_OFF;
			//}
		//}
		
				
		if (timerFlag == 0){	// timer flag is true every 1/2 second
			temp += readTemp;	// sums gathered temperatures
			numTemps++;			// keep track of the number of temperatures gathered
			//timerFlag = 0;		// reset the 1/4 second timer flag
		}
		
		if (timerFlag == 1){
			avgTemp = temp / numTemps;	// average the gathered temperature readings
			temp = 0;					// reset the summed temperatures
			numTemps = 0;				// reset the number of gathered temperatures
			timerFlag = 0;
			
			if (((avgTemp*10)/10 >= targetTemp+1.5) || ((avgTemp*10)/10 <= targetTemp-1.5)){ // if current temperature is outside of +/- 1.5C range
				count_ms = 0; //reset counter if temp is outside of +/- 1.5C range
				PORTD = LED_OFF;
			}
			else{
				PORTD = LED_ON;
			}
			
			
				
		}
		
		if (printFlag == 4){
			printStr("Temp is ");
			int_to_char(avgTemp);
						
			printStr(" and Target temp is ");
			int_to_char(targetTemp);
			uart_putchar('\n');
			printFlag = 0;
		}
			
		//isBatLow();
							
		
	}		
	
	OCR0A = HEAT_OFF;		// set heater to 0% duty cycle
	PORTB = Fan_OFF;	// power off the fan
		
		
}			


void ValidateData(void){
	char getValue = 'a';		// Receive Variable
	char newLine = '*';
	int valid = 0;			// Control Variable
	int i;
	int j;
	// Zero fill temp/time buffer:
	for (i =0; i<30; i++)
	{
		passData[i]='0';
	}
	i=0;
	char istrue ='Y';	// Hard coding handshake to validate data ***for testing***
	
	
	// For testing:
	printStr("Entered Validate Data Function \n"); // Debugg statement ***for testing***
	_delay_ms(2000);
	
	while(valid == 0){				// Control Structure for validating data values:
		printStr("Input is \n");	// Debugg statement ***for testing***
		//_delay_ms(2000);
		
		while (getValue != newLine)		// Takes in all 18 values send and confirms that the values are correct:
		{
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[i] = getValue;		// Put value into array.
			i++;						// Shift array.
		}
		uart_putchar('\n');				// After all the char are send needs a delimiter.
		_delay_ms(5000);
		
		/*	// Manual setting of array  ***for testing***
			//preheat:
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[0] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[1] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[2] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[3] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[4] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[5] = getValue;		// Put value into array.
			
			//Den
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[6] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[7] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[8] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[9] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[10] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[11] = getValue;		// Put value into array.
			
			//Ann
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[12] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[13] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[14] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[15] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[16] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[17] = getValue;		// Put value into array.
			
			//elo
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[18] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[19] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[20] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[21] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[22] = getValue;		// Put value into array.
			getValue = uart_getchar();	// Receive 6 values with 3 char per value.
			uart_putchar(getValue);		// Transmit them back to the App.
			passData[23] = getValue;		// Put value into array.
		
		uart_putchar('\n');				// After all the char are send needs a delimiter.
		_delay_ms(5000);
		*/
		
		printStr("Array is? \n"); // Debugg statement ***for testing***
		_delay_ms(2000);
		
		// Validating that data in array is same as what was passed. Debugg statements ***for testing***
		for (j=0; j<24; j++)	// 18 for no pre-heat & 24 for with pre-heat
		{
			uart_putchar(passData[j]);	
		}
		uart_putchar('\n');				// After all the char are send needs a delimiter.
	    _delay_ms(5000);				// Pause for app to check
		
		// Validating with User that times and temperatures are correct.
		getValue = 'Y';
		//printStr("Is data correct? \n"); // Communication with user
		//_delay_ms(3000);
		//getValue = uart_getchar();		// Receive "Y" for correct, else re-enter set PCR.
		valid = 1; // Testing
		if (getValue == istrue)			// Check with app if values were valid.
		{
			valid = 1;					// Exit value for control structure.
		}
	}
}

void set_PCR(void){
	// Control Variable
	int hundreds;
	int tens;
	int ones;
	
	printStr("In set array is \n"); //send
	_delay_ms(3000);				// Pause for app to check
	for (int j=0; j<24; j++) // 18 for no pre-heat & 25 for with pre-heat
	{
		uart_putchar(passData[j]);
	}
	uart_putchar('\n');				// After all the char are send needs a delimiter.
	_delay_ms(2000);				// Pause for app to check
	
	///* Function with Pre-heat option:
	// Set Preheat Temperature:
	hundreds = passData[0];
	tens = passData[1];
	ones = passData[2];
	hundreds = hundreds - '0';
	tens= tens - '0';
	ones = ones - '0';
	
	preheatTemp = hundreds * 100 + tens * 10 + ones;	// Shifts hundreds place value into time variable.

	printStr("Preheat temp is "); //send App
	hundreds = hundreds + '0';
	tens= tens + '0';
	ones = ones + '0';
	uart_putchar(hundreds);
	uart_putchar(tens);
	uart_putchar(ones);
	uart_putchar('\n');
	_delay_ms(2000);
	
	// Set Preheat Time:
	hundreds = passData[3];
	tens = passData[4];
	ones = passData[5];
	hundreds = hundreds - '0';
	tens= tens - '0';
	ones = ones - '0';
	
	preheatTime = hundreds * 100 +tens * 10 + ones;	// Shifts hundreds place value into time variable.

	printStr("Preheat time is "); //send App
	hundreds = hundreds + '0';
	tens= tens + '0';
	ones = ones + '0';
	uart_putchar(hundreds);
	uart_putchar(tens);
	uart_putchar(ones);
	uart_putchar('\n');
	_delay_ms(2000);
	
	
	// Set Denaturation Temperature:
	hundreds = passData[6];
	tens = passData[7];
	ones = passData[8];
	hundreds = hundreds - '0';
	tens= tens - '0';
	ones = ones - '0';
	
	denTemp = hundreds * 100 + tens * 10 + ones;	// Shifts hundreds place value into time variable.
	
	printStr("Denaturing temp is "); //send App
	hundreds = hundreds + '0';
	tens= tens + '0';
	ones = ones + '0';
	uart_putchar(hundreds);
	uart_putchar(tens);
	uart_putchar(ones);
	uart_putchar('\n');
	_delay_ms(2000);
	
	// Set Denaturation Time:
	hundreds = passData[9];
	tens = passData[10];
	ones = passData[11];
	hundreds = hundreds - '0';
	tens= tens - '0';
	ones = ones - '0';
	
	denTime = hundreds * 100 +tens * 10 + ones;	// Shifts hundreds place value into time variable.
	
	printStr("Denaturing time is "); //send App
	hundreds = hundreds + '0';
	tens= tens + '0';
	ones = ones + '0';
	uart_putchar(hundreds);
	uart_putchar(tens);
	uart_putchar(ones);
	uart_putchar('\n');
	_delay_ms(2000);
	
	
	// Set Annealing Temperature:
	hundreds = passData[12];
	tens = passData[13];
	ones = passData[14];
	hundreds = hundreds - '0';
	tens= tens - '0';
	ones = ones - '0';
	
	annTemp = hundreds * 100 +tens * 10 + ones;	// Shifts hundreds place value into time variable.
	
	printStr("Annealing temp is "); //send App
	hundreds = hundreds + '0';
	tens= tens + '0';
	ones = ones + '0';
    uart_putchar(hundreds);
    uart_putchar(tens);
    uart_putchar(ones);
    uart_putchar('\n');
    _delay_ms(2000);
	
	
	// Set Annealing Time:
	hundreds = passData[15];
	tens = passData[16];
	ones = passData[17];
	hundreds = hundreds - '0';
	tens= tens - '0';
	ones = ones - '0';
	
	annTime = hundreds * 100 +tens * 10 + ones;	// Shifts hundreds place value into time variable.

	printStr("Annealing time is "); //send App
	hundreds = hundreds + '0';
	tens= tens + '0';
	ones = ones + '0';
    uart_putchar(hundreds);
    uart_putchar(tens);
    uart_putchar(ones);
    uart_putchar('\n');
    _delay_ms(2000);
	
	
	// Set Elongation Temperature:
	hundreds = passData[18];
	tens = passData[19];
	ones = passData[20];
	hundreds = hundreds - '0';
	tens= tens - '0';
	ones = ones - '0';
	
	eloTemp = hundreds * 100 +tens * 10 + ones;	// Shifts hundreds place value into time variable.
	
	printStr("Elongation temp is "); //send App
	hundreds = hundreds + '0';
	tens= tens + '0';
	ones = ones + '0';
    uart_putchar(hundreds);
    uart_putchar(tens);
    uart_putchar(ones);
    uart_putchar('\n');
    _delay_ms(2000);
	
	
	// Set Elongation Time:
	hundreds = passData[21];
	tens = passData[22];
	ones = passData[23];
	hundreds = hundreds - '0';
	tens= tens - '0';
	ones = ones - '0';
	
	eloTime = hundreds * 100 +tens * 10 + ones;	// Shifts hundreds place value into time variable.
	
	printStr("Elongation time is "); //send App
	hundreds = hundreds + '0';
	tens= tens + '0';
	ones = ones + '0';
    uart_putchar(hundreds);
    uart_putchar(tens);
    uart_putchar(ones);
    uart_putchar('\n');
    _delay_ms(2000);
	//*/
	/* Without pre-heat
	// Set Denaturation Temperature:
	hundreds = passData[0];
	tens = passData[1];
	ones = passData[2];
	hundreds = hundreds - '0';
	tens= tens - '0';
	ones = ones - '0';
	
	denTemp = hundreds * 100 + tens * 10 + ones;	// Shifts hundreds place value into time variable.
	
	printStr("Denaturing temp is "); //send App
	hundreds = hundreds + '0';
	tens= tens + '0';
	ones = ones + '0';
	uart_putchar(hundreds);
	uart_putchar(tens);
	uart_putchar(ones);
	uart_putchar('\n');
	_delay_ms(2000);
	
	// Set Denaturation Time:
	hundreds = passData[3];
	tens = passData[4];
	ones = passData[5];
	hundreds = hundreds - '0';
	tens= tens - '0';
	ones = ones - '0';
	
	denTime = hundreds * 100 +tens * 10 + ones;	// Shifts hundreds place value into time variable.
	
	printStr("Denaturing time is "); //send App
	hundreds = hundreds + '0';
	tens= tens + '0';
	ones = ones + '0';
	uart_putchar(hundreds);
	uart_putchar(tens);
	uart_putchar(ones);
	uart_putchar('\n');
	_delay_ms(2000);
	
	
	// Set Annealing Temperature:
	hundreds = passData[6];
	tens = passData[7];
	ones = passData[8];
	hundreds = hundreds - '0';
	tens= tens - '0';
	ones = ones - '0';
	
	annTemp = hundreds * 100 +tens * 10 + ones;	// Shifts hundreds place value into time variable.
	
	printStr("Annealing temp is "); //send App
	hundreds = hundreds + '0';
	tens= tens + '0';
	ones = ones + '0';
	uart_putchar(hundreds);
	uart_putchar(tens);
	uart_putchar(ones);
	uart_putchar('\n');
	_delay_ms(2000);
	
	
	// Set Annealing Time:
	hundreds = passData[9];
	tens = passData[10];
	ones = passData[11];
	hundreds = hundreds - '0';
	tens= tens - '0';
	ones = ones - '0';
	
	annTime = hundreds * 100 +tens * 10 + ones;	// Shifts hundreds place value into time variable.

	printStr("Annealing time is "); //send App
	hundreds = hundreds + '0';
	tens= tens + '0';
	ones = ones + '0';
	uart_putchar(hundreds);
	uart_putchar(tens);
	uart_putchar(ones);
	uart_putchar('\n');
	_delay_ms(2000);
	
	
	// Set Elongation Temperature:
	hundreds = passData[12];
	tens = passData[13];
	ones = passData[14];
	hundreds = hundreds - '0';
	tens= tens - '0';
	ones = ones - '0';
	
	eloTemp = hundreds * 100 +tens * 10 + ones;	// Shifts hundreds place value into time variable.
	
	printStr("Elongation temp is "); //send App
	hundreds = hundreds + '0';
	tens= tens + '0';
	ones = ones + '0';
	uart_putchar(hundreds);
	uart_putchar(tens);
	uart_putchar(ones);
	uart_putchar('\n');
	_delay_ms(2000);
	
	
	// Set Elongation Time:
	hundreds = passData[15];
	tens = passData[16];
	ones = passData[17];
	hundreds = hundreds - '0';
	tens= tens - '0';
	ones = ones - '0';
	
	eloTime = hundreds * 100 +tens * 10 + ones;	// Shifts hundreds place value into time variable.
	
	printStr("Elongation time is "); //send App
	hundreds = hundreds + '0';
	tens= tens + '0';
	ones = ones + '0';
	uart_putchar(hundreds);
	uart_putchar(tens);
	uart_putchar(ones);
	uart_putchar('\n');
	_delay_ms(2000);
	
	*/
}

uint8_t userInput(void){
	char getInput;
	char stopPRC = '*';
	//char handshake = "~";
	getInput = uart_getchar(); // Get input from User.
	
	// Special Handling for User Stop PCR:
	if (getInput == stopPRC){ // App sends a "*" if the user requests Stop PCR
		stopPCR();
		/*
		while (1){
			printStr("PCR has been Stopped \n"); //send
			//_delay_ms(2000);
			printStr("Please turn off PCR device \n"); //send
			//_delay_ms(2000);
		}
		*/
	}
	return getInput;
}	

void runPCR(void){
	int cycle = 0;
	
	//for (cycle=0; cycle < 30; cycle++)
	while(cycle < 30)
	{
		printStr("In cycle ");
		int_to_char(cycle);
		uart_putchar('\n');
		//printStr("In run PCR \n"); //send
		holdAtTemp(denTemp,denTime);
		//printStr("passed den \n"); //send
		holdAtTemp(annTemp,annTime);
		//printStr("passed ann \n"); //send
		holdAtTemp(eloTemp,eloTime);
		//printStr("passed elo \n"); //send
		cycle = cycle + 1;
	}
}