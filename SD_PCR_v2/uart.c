

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define BAUDRATE 9600
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)

// Libraries:
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include <stdlib.h>
#include "uart.h"

char buffer[90];
/*
SERIAL COMMUNICATION for USART 1 on Atmega32u4
*/
	
/*
****UBRR1H
****UBRR1L

****USART Control and Status Register A – UCSR1A
	RXC1: USART Receive Complete
	TXC1: USART Transmit Complete
	UDRE1: USART Data Register Empty
	FE1: Frame Error
	DOR1: Data OverRun
	UPE1: USART Parity Error
	U2X1: Double the USART Transmission Speed
	MPCMn: Multi-processor Communication Mode
	
****USART Control and Status Register B – UCSR1B
	RXCIE1: RX Complete Interrupt Enable 
	TXCIE1: TX Complete Interrupt Enable 
	UDRIE1: USART Data Register Empty Interrupt Enable 
	RXEN1: Receiver Enable 
	TXEN1: Transmitter Enable 
	UCSZ12: Character Size 
	RXB81: Receive Data Bit 8 
	TXB81: Transmit Data Bit 8
	
****USART Control and Status Register C – UCSR1C
	UMSEL11: USART Mode Select
	UMSEL10: USART Mode Select
	00 - Asynchronous USART
	01 - Synchronous UASRT
	10 - Reserved
	11 - SPI master mode
	
	UPM11: Parity Mode
	UPM10: Parity Mode
	00 - Disabled
	01 - Reserved
	10 - Enable, Even parity
	11 - Enable, Odd parity
	
	USBS1: Stop Bit Select
	0 - 1-bit
	1 - 2-bits
	
	UCSZ12: Character Size **not in this register** is set to 0 by default
	UCSZ11: Character Size
	UCSZ10: Character Size
	000 - 5-bit
	001 - 6-bit
	010 - 7-bit
	011 - 8 bit
	100 - reserved
	101 - reserved
	110 - reserved
	111 - 9-bit
	
	UCPOL1: Clock Polarity
	0 - transmitted rising edge, received falling
	1 - transmitted falling edge, received rising
	
****USART Control and Status Register D – UCSR1D
	Bits 7:2 – Reserved bits
	CTSEN: UART CTS Signal Enable
	RTSEN: UART RTS Signal Enable
*/


/*
The USART has to be initialized before any communication can take place. The initialization process normally
consists of setting the baud rate, setting frame format and enabling the Transmitter or the Receiver depending
on the usage.
*/
void uart_init(void){
	// Set Baud Rate:
	UBRR1H = (uint8_t)(BAUD_PRESCALLER>>8);
	UBRR1L = (uint8_t)(BAUD_PRESCALLER);
	
	//USART Control and Status Register A – UCSR1A   ***Do not need to maniplulate****
	//UCSR1A = ((0<<RXC1)|(0<<TXC1)|(0<<UDRE1)|(0<<FE1)|(0<<DOR1)|(0<<UPE1)|(0<<U2X1)|(0<<MPCM1))
	
	//USART Control and Status Register B – UCSR1B	
	UCSR1B = ((0<<RXCIE1)|(0<<TXCIE1)|(0<<UDRIE1)|(1<<RXEN1)|(1<<TXEN1)|(0<<UCSZ12)|(0<<RXB81)|(0<<TXB81)); // Enable transmit and receive  
	
	//USART Control and Status Register C – UCSR1C
	UCSR1C = ((0<<UMSEL11)|(0<<UMSEL10)|(0<<UPM11)|(0<<UPM10)|(0<<USBS1)|(1<<UCSZ11)|(1<<UCSZ10)|(0<<UCPOL1)); // asynchronous, no parity, 1 stop, 8 data, transmitted rising edge & received falling

	//USART Control and Status Register D – UCSR1D   ***Do not need to maniplulate****
	//UCSR1D = ((0<<CTSEN)|(0<<RTSEN));
	
	// buffer configure to all zeros:
	int i;
	for (i =0; i<90; i++)
	{
		buffer[i]='0';
	}
}

// Transmit a byte, after check to make sure line is clear:
void uart_putchar(unsigned char data)
{
 while (!(UCSR1A & (1 << UDRE1))); //UDRE1: USART Data Register Empty
 UDR1 = data;
}

// Receive a byte, after checking to make sure there is a byte available:
unsigned char uart_getchar(void)
{
       while(!(UCSR1A & (1<<RXC1))); // RXC1: USART Receive Complete
       DDRD |= (1<<6);
       PORTD ^= (1<<6);
       return UDR1;
}

//send a string of characters
void printStr(char* data){

	uint8_t i=0;
	//send each character until we hit the end of the string
	while(data[i] != '\0'){
		uart_putchar(data[i]);
		i++;
	}
}

void int_to_char(int in){
	char ones, tens, huns;
	int temp1;
	int temp2;
	int temp3;
	temp1 = in/100;
	huns = (char)temp1;
	huns = huns + '0';
	temp2 = (in-temp1*100)/10;
	tens = (char)temp2 + '0';
	temp3 = (in-temp1*100-temp2*10);
	ones = (char)temp3 + '0';
	
	uart_putchar(huns);
	uart_putchar(tens);
	uart_putchar(ones);
	//uart_putchar('\n');
}