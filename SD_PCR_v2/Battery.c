/* Battery.c
 *  Author: martijus
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL
#endif
				
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>		// For _ms_delay function
#include "uart.h"
#include "ADC.h"
#include "Timer.h"

// Battery Protection:
#define Batt_low	(PINF	&= ~(1<<PF1))	// Check if Pin F1 is low, then low battery detected.
#define Relay_off	(PORTB  |= (1<<PB1))	// Toggle Relay on.
#define Relay_on	(PORTB  &= ~(1<<PB1))	// Toggle Relay off.
#define isRelay_off	(PINB	|= (1<<PB1))	// Check if Pin B1 is high, then Relay is on (e.g. H&C is in cut-off)
#define isRelay_on	(PINB	&= ~(1<<PB1))	// Check if Pin B1 is low, then Relay is off (e.g. H&C is on)
#define LED_ON		(PORTD	|= (1<< PD6))	// Toggle Teensy LED on.
#define LED_OFF		(PORTD	&= ~(1<<PD6))	// Toggle Teensy LED off.

void isBatLow(void){
	if (PINF == Batt_low)			// Low battery level detected. Getting 0 from sensor
	{
		if (PINB == isRelay_off)	// Check to see if relay is already on. Relay on is 5v
		{
			PORTB = Relay_on;		// When the relay turns on, the power to the heating & Cooling block is disconnected.
			PORTD = LED_ON;		// for testing function
		}
	}
	
	if (PINF != Batt_low)		// Low battery level NOT detected.
	{
		PORTB = Relay_off;		// When there is no low battery detected the power to the heating & cooling block is connected.
		PORTD = LED_OFF;		// for testing function
	}
	
}

void stopPCR(void){
	while (1)
	{
		PORTB = Relay_on;		// When there is no low battery detected the power to the heating & cooling block is connected.
		
		// For testing
		PIND = LED_ON;
		_delay_ms(1000);
		PIND = LED_OFF;
		printStr("Battery is low PCR has shut down. \n"); //send
		_delay_ms(5000);
		printStr("Please turn off PCR and charge battery. \n"); //send
	}
}