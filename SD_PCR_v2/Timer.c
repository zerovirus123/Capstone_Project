/*Timer.c
 *  Author: martijus
 Implementing a Clear Timer on Compare (CTC) Mode timer/counter.
 The comparison takes place in the hardware itself, inside the AVR CPU.
 Once the process value becomes equal to the set point, a flag in
 the status register is set and the timer is reset automatically.
 To implement you set the flag to the time you wish to count.
 Steps:
 1) setup CPU speed, with #define statements, and putting CPU_PRESCALE(F_CPU) in main.
 2) Setup register configuration function: void Counter_init (void);.
 3) Tell the CPU what to do when interrupt flag happens, ISR().
 4) In main enable interrupts with sei() function, and set at what count you want the interrupt to happen at with the Output Compare Register OCR=__;
 */ 


#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/*
avr/io.h Allows STD input output functions

interrupt.h Allows use of functions: sei() {enable interrupt}, cli() {disable interrupt},
and ISR definitions {Interrupt service routine, e.g. what happens when an interrupt flag is set}
*/
#include <avr/io.h>
#include <avr/interrupt.h>


//**********Global Variables**********
//volatile unsigned int count_ms = 0;



/* CTC uses both TCCR registers, Waveform Generation Mode (WGM1_, where _={0,1,2,3}) are four bits, two in each register.
	There are two CTC modes WGM13 WGM12 WGM11 WGM10: 
	{0100} uses Flag OC1A, Output Compare Register 1 A
	{1100} uses Flag ICR1, Input Capture Register 1
	*/

void timer_init (void)
{
	/*
	Timer/Counter1 Control Register A = TCCR1A
	TCCR1A Bit [7:0] {COM1A1,COM1A0,COM1B1,COM1B0,COM1C1,COM1C0,WGM11,WGM10}
		COM1A1: Compare Output Mode for Channel A
		COM1A0: Compare Output Mode for Channel A
		COM1B1: Compare Output Mode for Channel B
		COM1B0: Compare Output Mode for Channel B
		COM1C1: Compare Output Mode for Channel C
		COM1C0: Compare Output Mode for Channel C
		WGM11: Waveform Generation Mode
		WGM10: Waveform Generation Mode
	*/
		
	TCCR1A |= 0x00;	//Setting TCCR1A Mode to CTC. Since bit 0 (WGM10), and bit1 (WGM11) are set to zero for both CTC setting we can set TCCR0A = 0000 0000

	/*  
	Timer/Counter1 Control Register B = TCCR1B
	TCCR1B bit [7:0] {ICNC1,ICES1,Reserved,WGM13,WGM12,CS12,CS11,CS10}
		ICNC1: Input Capture Noise Canceler
		ICES1: Input Capture Edge Select
		Reserved: Reserved Bit
		WGM13: Waveform Generation Mode
		WGM12: Waveform Generation Mode
		CS12: Clock Select
		CS11: Clock Select
		CS10: Clock Select
	Clock Select Bit Description: [2:0]
		000 = clock off
		001 = CPU/1 no prescaling
		010 = CPU/8
		011 = CPU/64 
		100 = CPU/256
		101 = CPU/1024
		110 = External clock source on Tn pin. Clock on falling edge
		111 = External clock source on Tn pin. Clock on rising edge
	*/
	TCCR1B |= ((1<<CS12)|(1<<WGM12)); //Setting TCCR1B initial value to 256 prescaler & setting CTC output mode (ATMEGA32U4 datasheet p.133, p.331 respectively). TCCR1B = 0000 1010
	
	/*
	Timer/Counter1 Interrupt Mask Register 1 = TIMSK1
	TIMSK1 bit [7:0] {Reserved,Reserved,ICIE1,Reserved,OCIE1C,OCIE1B,OCIE1A,TOIE1}
		Reserved: Reserved Bit
		Reserved: Reserved Bit
		ICIE1: ICIEn: Timer/Countern, Input Capture Interrupt Enable
		Reserved: Reserved Bit
		OCIE1C: OCIEnC: Timer/Countern, Output Compare C Match Interrupt Enable
		OCIE1B: OCIEnB: Timer/Countern, Output Compare B Match Interrupt Enable
		OCIE1A: OCIEnA: Timer/Countern, Output Compare A Match Interrupt Enable
		TOIE1: TOIEn: Timer/Countern, Overflow Interrupt Enable
	*/
	TIMSK1 |=(1<<OCIE1A); // The OCIE1A bit enables interrupt flag.
	
	/*
	Output Compare Register 1A (OCR1A) value must be [0:65535], because the TCCR registers for TCCR1 are 16 bit 2^16 = 65536. 
	Calculating your count: OCR1A = (CPU_clock speed/TCCR_clock speed)-1
	*OCR1A is being set to count in milliseconds*  
	*/
	OCR1A = 31248;  //timer 1/2 second // timer compare value   8MHz/8 = 1MHZ = 1 second (out of range), and 1000 = 1ms
	
	
	TCNT1 = 0; // Initiates the timer to count CPU cycles.
}
// Code without comments or main example:
/*
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define CPU_8MHz        0x01
#include <avr/io.h>
#include <avr/interrupt.h>
volatile unsigned int count_ms = 0;
void Counter_init (void);
ISR(TIMER1_COMPA_vect){
	count_ms ++;
}

int main(void)
{
	CPU_PRESCALE(CPU_8MHz); 
	Counter_init();
	sei();
	while(1)
	{
		// Code goes here
	}
}


void Counter_init (void)
{
	TCCR1A |= 0x00;
	TCCR1B |= ((1<<CS11)|(1<<WGM12));
	TIMSK1 |=(1<<OCIE1A);
	OCR1A = 1000;
	TCNT1 = 0;
}
*/

void pwm_init(void){
	
	TCCR0A = (1<<COM0A1)|(1<<COM0A0)|(1<<WGM01)|(1<<WGM00); //Fast PWM mode, clear on compare match
	TCCR0B = (0<<WGM02)|(1<<CS02)|(0<<CS01)|(1<<CS00); // pre-scaler 1024
	DDRB |= (DDRB |= (1<<PB7)); //sets PORTB Pin 7 (OCOA -- Timer/Counter0 output) as an output pin for PWM
	OCR0A = 255; //default output to 0% duty cycle
}