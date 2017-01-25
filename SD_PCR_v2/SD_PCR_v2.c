/* SD_PCR.c
 *  Author: martijus
 */ 
#ifndef CPU_PRESCALE
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#endif

#ifndef CPU_16MHz
#define CPU_16MHz        0x00
#endif

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>	
#include <util/delay.h>		// For _ms_delay function
#include "uart.h"			// Controls data transfer between the Bluetooth (App) and Teensy.
#include "ADC.h"			// Controls read thermistor values into the Teensy
#include "Timer.h"			// Controls PWM and ISR counter
#include "Battery.h"		// Controls battery cut-off 
#include "H_C.h"

/////////// Begin Definitions: ////////////////////////////////////////////
#define LED_ON		(PORTD	|= (1<< PD6))	// Toggle Teensy LED on.
#define LED_OFF		(PORTD	&= ~(1<<PD6))	// Toggle Teensy LED off.

#define baud 9600							// UART
						
/////////// END Definitions: ////////////////////////////////////////////


// Main Function Decelerations:
void set_init(void);	// Set data direction for ports and initial values

///////////////////////////////////	Begin Main	/////////////////////////////////////////////

int main(void)
{
	// Initializations:
	set_init();
	printStr("Welcome to PCR \n");
	_delay_ms(1000);
	
	//isBatLow();
	char input;
	ValidateData();
	input = 1;	// for testing bypass user input.
	
    while(1)
    {
        //PORTD ^= 1<<6; //toggle LED to know we're alive
		/* Control Structure:
		1. Set PCR
		2. Run PCR
		*/

		printStr("Press 1 to set PCR \n"); //send 
		_delay_ms(2000);
			
		printStr("Press 2 to run PCR \n"); //send
		_delay_ms(2000);
		

		//input = uart_getchar();	// Get input from User.
		//printStr("command is "); //send
		//_delay_ms(1000);
		//printStr (input);
		//_delay_ms(5000);
		//input= input -'0';		// Convert to char to int
		
		
		
		switch ( input ) {
			case 1:
			//ValidateData();
			set_PCR();
			printStr("PCR has been set. \n"); //send 
			_delay_ms(1000);
			signed char valid;
			valid = '~';
			UDR1 = valid;
			uart_putchar (valid);
			printStr("sent ~ \n");
			_delay_ms(1000);
			input = 2;	// for testing bypass user input.
			break;
			
			case 2:
			
			printStr("Run PCR \n"); //send
			
			runPCR();
			//input = run;
			printStr("PCR finished \n"); //send
			input = 10;	// for testing bypass user input.
			break;
			
			default:
			//powerSAVE();
			break;
		
		}
			
		//isBatLow();
		
	}
	
}



void set_init(void){
	CPU_PRESCALE(CPU_16MHz);	//Set up UART
	timer_init();
	pwm_init();
	adc_init();
	uart_init();
	char clear;
	while (clear != '\0')		// Takes in all 18 values send and confirms that the values are correct:
	{
		clear = uart_getchar();
	}
	//uart_init2();
	h_c_init(); 
	
	PORTB = 0b00000000; // Sets all PB pin values low.
	PORTF = 0b00000000; // Sets all PF pins low.
	PORTD = 0b00000000; // Sets all PD pins low.
	sei(); // Enable interrupts
}	

