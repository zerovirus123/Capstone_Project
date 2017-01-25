/* Timer.h
 * Author: martijus
 * Implementing a Clear Timer on Compare (CTC) Mode timer/counter.
 The comparison takes place in the hardware itself, inside the AVR CPU.
 Once the process value becomes equal to the set point, a flag in
 the status register is set and the timer is reset automatically.
 To implement you set the flag to the time you wish to count.
 Steps:
 1) setup CPU speed, with #define statements, and putting CPU_PRESCALE(F_CPU) in main.
 2) Setup register configuration function: void Counter_init (void);.
 3) Tell the CPU what to do when interrupt flag happens, ISR().
 4) In main enable interrupts with sei() function, and set at what count you want the interrupt to happen at with the Output Compare Register OCR=__;
 * 
 */ 

#ifndef _Timer_h_
#define _Timer_h_

//**********function declarations*********
/*
Initiation Function:
TCCR1A |= 0x00; Setting TCCR1A Mode to CTC. Since bit 0 (WGM10), 
and bit1 (WGM11) are set to zero for both CTC setting we can set TCCR0A = 0000 0000

TCCR1B |= ((1<<CS11)|(1<<WGM12)); //Setting TCCR1B initial value to 256 prescaler 
& setting CTC output mode (ATMEGA32U4 datasheet p.133, p.331 respectively). TCCR1B = 0000 1010

TIMSK1 |=(1<<OCIE1A); // The OCIE1A bit enables interrupt flag.

OCR1A = 1000;   // timer compare value   8MHz/8 = 1MHZ = 1 second (out of range), and 1000 = 1ms

TCNT1 = 0; // Initiates the timer to count CPU cycles.
*/
void timer_init (void);
void pwm_init(void);

#endif