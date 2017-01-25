/*ADC.h
 * Author: martijus
 *
 * 
 *  
 */ 

#ifndef _ADC_h_
#define _ADC_h_

//**********function declarations*********

/*
adc_init Function:
DIDR0 = 0x01;  // 00000001 is the binary equivalent of 0x01. 
Setting the register to 00010011 disables digital input, making the pin an analog pin. ADC pin 0 is F0.

ADMUX |= (1 << REFS0); // 01000000 sets the register to keep right adjust on, 
and selects ADC0 and only input channel. 

ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);Setting the register to 11000111. 
Enables ADC and configures ADC prescaler to a 128 division factor. 
*/
void adc_init(void);

/*
adc_read Function:
ADSC bit in the ADCSRA register is toggled high to initiate an ADC read.
The ADATE bit in register ADCSRA is an auto trigger enable for the register. When conversion is complete it will go low.
After getting a valid read you have to toggle the interrupt flag ADIF in the ADCSRA register low again.
We create the dummy variable low because if we set data= ((ADCH<<8) | ADCL); the register will read the lower bits first.
for our reading we need it to see the ADCH bits first. By setting the low bits into an int we can get the compiler to set
all the bits in the correct order.
ADC has two 8 bit registers to take in a reading, ADCH for the high bits and ADCL for the low bits.
*/
uint16_t adc_read(void);


/*
convertADC Function:
This function takes the ADC reading and does the following:
1) Finds the voltage reading from the ADC 1024 divisions of Vref in ADC reading.
2) Using the voltage divider equation find the resistance associated with the voltage reading.
3) Converts the into the equivalent temperature based on the thermistors chart.
4) Adds an off-set to chart to account for the real temp for "this" thermistor.
5) Returns temperature reading.

*/
double convertADC(double);

/*
convertTemp Function:
This function takes the ADC reading and does the following:
1) Compares resistance value to the thermistors 5°C look-up chart.
2) When within a range matches resistance to a 5 division linearization to get exact 1°C temperature. 
3) returns temperature approximation. 
*/
double convertTemp(double);

#endif