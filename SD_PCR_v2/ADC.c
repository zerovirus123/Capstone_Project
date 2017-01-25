

// Create definitions for UART
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define BAUD_RATE 9600

// Libraries:
#include <avr/io.h>
#include <avr/interrupt.h>	// for UART
#include <util/delay.h>		// for ms_delay
#include <math.h>			// for exp()

// Functions:
void adc_init();
uint16_t adc_read(void);
double convertADC(double);
double convertTemp(double);




void adc_init() {
	/* Digital Input Disable Register 0 – DIDR0
	Bits {7:4, 1:0}	- When these bits are written logic one, the digital input buffer on the corresponding ADC pin is disabled.
	The corresponding PIN Register bit will always read as zero when this bit is set.
	0 - ADC0D -> F0
	1 - ADC1D -> F1
	2 - NULL
	3 - NULL
	4 - ADC4D -> F4
	5 - ADC5D -> F5
	6 - ADC6D -> F6
	7 - ADC7D -> F7	 
	*/
	DIDR0 = 0x01;  // 00000001 is the binary equivalent of 0x01. Setting the register to 00010011 disables digital input, making the pin an analog pin. ADC pin 0 is F0.
	
	/*ADC Multiplexer Selection Register – ADMUX
	Bits {7:0}
	7 - REFS1: select the voltage reference for the ADC
	6 - REFS0: select the voltage reference for the ADC
	5 - ADLAR: ADC Left Adjust Result. The ADLAR bit affects the presentation of the ADC conversion result in the ADC Data Register.
		Write one to ADLAR to left adjust the result. Otherwise, the result is right adjusted. Changing the
		ADLAR bit will affect the ADC Data Register immediately, regardless of any ongoing conversions.
	4 - Analog Channel Selection Bit
	3 - Analog Channel Selection Bit
	2 - Analog Channel Selection Bit
	1 - Analog Channel Selection Bit
	0 - Analog Channel Selection Bit
	Bits 4:0 – MUX4:0: Analog Channel Selection Bits (See p.308 of data sheet for table)  
	The value of these bits selects which combination of analog inputs are connected to the ADC.
	These bits also select the gain for the differential channels. If these bits are changed during a conversion, 
	the change will not go in effect until this conversion is complete (ADIF in ADCSRA is set).
	*/	
	ADMUX |= (1 << REFS0); // 01000000 sets the register to keep right adjust on, and selects ADC0 and only input channel. 
	
	
	/*ADC Control and Status Register A – ADCSRA
	Bits {7:0}
		7 - ADEN: ADC Enable. 
			Writing this bit to one enables the ADC. By writing it to zero, the ADC is turned off. Turning the
			ADC off while a conversion is in progress, will terminate this conversion.
		6 - ADSC: ADC Start Conversion
			In Single Conversion mode, write this bit to one to start each conversion. In Free Running mode,
			write this bit to one to start the first conversion. The first conversion after ADSC has been written
			after the ADC has been enabled, or if ADSC is written at the same time as the ADC is enabled,
			will take 25 ADC clock cycles instead of the normal 13. This first conversion performs initialization
			of the ADC.
			ADSC will read as one as long as a conversion is in progress. When the conversion is complete,
			it returns to zero. Writing zero to this bit has no effect.
		5 - ADATE: ADC Auto Trigger Enable
			When this bit is written to one, Auto Triggering of the ADC is enabled. The ADC will start a conversion
			on a positive edge of the selected trigger signal. The trigger source is selected by setting
			the ADC Trigger Select bits, ADTS in ADCSRB.
		4 - ADIF: ADC Interrupt Flag
			This bit is set when an ADC conversion completes and the Data Registers are updated. The
			ADC Conversion Complete Interrupt is executed if the ADIE bit and the I-bit in SREG are set.
			ADIF is cleared by hardware when executing the corresponding interrupt handling vector. Alternatively,
			ADIF is cleared by writing a logical one to the flag. Beware that if doing a Read-Modify-Write on ADCSRA, 
			a pending interrupt can be disabled. This also applies if the SBI and CBI instructions are used.
		3 - ADIE: ADC Interrupt Enable
			When this bit is written to one and the I-bit in SREG is set, the ADC Conversion Complete Interrupt
			is activated.
		2 - ADC Prescaler Select Bits
		1 - ADC Prescaler Select Bits
		0 - ADC Prescaler Select Bits
		ADPS2:0: ADC Prescaler Select Bits
		These bits determine the division factor between the XTAL frequency and the input clock to the
		ADC. (See data sheet p.311 for table)
	*/
	// Setting the register to 11000111. Enables ADC and configures ADC prescaler to a 128 division factor. 
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(void){
	uint16_t data;
	uint8_t low;
	//_delay_ms(1000); // only reads every second. 
	/* ADSC bit in the ADCSRA register is toggled high to initiate an ADC read.
	   Since we have already selected F0 as our only ADC read pin the register
	   will read from that pin only. */ 
	ADCSRA |= (1<<ADSC); 
	
	/* Wait for a Valid Sample. 0x10 = 00010000 = ADATE  
	The ADATE bit in register ADCSRA is an auto trigger enable for the register. When conversion is complete it will go low.*/
	while ((ADCSRA & 0x10) == 0 );  
	
	/*After getting a valid read you have to toggle the interrupt flag ADIF in the ADCSRA register low again.
	  If we were using interrupts the flag would be reset automatically.  */
	ADCSRA |= (1<<ADIF); 
	
	/*We create the dummy variable low because if we set data= ((ADCH<<8) | ADCL); the register will read the lower bits first.
	for our reading we need it to see the ADCH bits first. By setting the low bits into an int we can get the compiler to set 
	all the bits in the correct order. */
	
	low = ADCL; // Must use to adjust bits in following equation. 
	//ADC has two 8 bit registers to take in a reading, ADCH for the high bits and ADCL for the low bits.
	data = ((ADCH<<8) | low);
	 
	return (data);
}


double convertADC(double data){
	// variables:
	double resistance;
	double Vref = 5;
	double voltage = 0;
	double temp;
	double offset=0;
	

	/* Vref = 5v, ADC divides 0-5v into 1024 (2^10) sections. Thus each ADC value is 5/1024=0.004833  
	*/
	voltage = data*0.004833; //1024 divisions of Vref in ADC reading
	resistance = (-10000* voltage)/(voltage -Vref); // rearange the voltage divider equation to get thermistor resistance.
	temp = convertTemp(resistance) + offset; // use look-up table to get temperatures and calibrate with off-set.
	return (temp);	// outputs conversion.
}




// finds temp in range, then uses equation fitting for 1°C accuracy. If not in range returns 256.
double convertTemp(double resist){	
	

	// -55°C to -50°C range:
	if (resist <= 526240 && resist > 384520)
	{
		return -55;
	}
	
	// -50°C to -45°C range:
	else if (resist <= 384520 && resist > 284010)
	{
		return -50;
	}
	
	// -45°C to -40°C range:
	else if (resist <= 284010 && resist > 211940)
	{
		return -45;
	}
	
	// -40°C to -35°C range:
	else if (resist <= 211940 && resist > 159720)
	{
		return -40;
	}
	
	// -35°C to -30°C range:
	else if (resist <= 159720 && resist > 121490)
	{
		return -35;
	}
	
	// -30°C to -25°C range:
	else if (resist <= 121490 && resist > 93246)
	{
		return -30;
	}
	
	// -25°C to -20°C range:
	else if (resist <= 93246 && resist > 72181)
	{
		return -25;
	}
	
	// -20°C to -15°C range:
	else if (resist <= 72181 && resist > 56332)
	{
		return -20;
	}
	
	// -15°C to -10°C range:
	else if (resist <= 56332 && resist > 44308)
	{
		return -15;
	}
	
	// -10°C to -5°C range:
	else if (resist <= 44308 && resist > 35112)
	{
		return -10;
	}
	
	// -5°C to 0°C range:
	else if (resist <= 35112 && resist > 28024)
	{
		return -5;
	}
	
	// 0°C to 5°C range:
	else if (resist <= 28024 && resist > 22520)
	{
		return 0;
	}
	
	// 5°C to 10°C range:
	else if (resist <= 22520 && resist > 18216)
	{
		return 5;
	}
	
	// 10°C to 15°C range:
	else if (resist <= 18216 && resist > 14827)
	{
		return 10;
	}
	
	// 15°C to 20°C range:
	else if (resist <= 14827 && resist > 12142)
	{
		return 15;
	}
	
	/*******************************************************************************************************************
	********************* IN PCR RANGE *********************************************************************************
	*******************************************************************************************************************/
	
	// 20°C to 25°C range:
	else if (resist <= 12142 && resist > 10000)
	{
		if (resist <= 12142 && resist > 11713.6)// 12142-10000 = 2142 & 2142/5 = 428.4
		{
		return 20;
		}
		else if (resist <= 11713.6 && resist > 11285.2)
		{
			return 21;
		}
		else if (resist <= 11285.2 && resist > 10856.8)
		{
			return 22;
		}
		else if (resist <= 10856.8 && resist > 10428.4)
		{
			return 23;
		}
		else if (resist <= 10428.4 && resist > 10000)
		{
			return 24;
		}
	}
	
	// 25°C to 30°C range:
	else if (resist <= 10000 && resist > 8281.8)
	{
		if (resist <= 10000 && resist > 9656.36) // 10000-8281.8 = 1718.2 & 1718.2/5 = 343.64
		{
		return 25;
		}
		else if (resist <= 9656.36 && resist > 9312.72)
		{
			return 26;
		}
		else if (resist <= 9312.72 && resist > 8969.08)
		{
			return 27;
		}
		else if (resist <= 8969.08 && resist > 8625.44)
		{
			return 28;
		}
		else if (resist <= 8625.44 && resist > 8281.8)
		{
			return 29;
		}
	}
	
	// 30°C to 35°C range:
	else if (resist <= 8281.8 && resist > 6895.4)
	{
		return 30 + (8281.8 - resist)/277.28;
		/*
		if (resist <= 8281.8 && resist > 8004.52) // 8281.8-6895.4= 1386.4 & 1386.4/5= 277.28
		{
		return 30;
		}
		else if (resist <= 8004.52 && resist > 7727.24)
		{
			return 31;
		}
		else if (resist <= 7727.24 && resist > 7449.96)
		{
			return 32;
		}
		else if (resist <= 7449.96 && resist > 7172.68)
		{
			return 33;
		}
		else if (resist <= 7172.68 && resist > 6895.4)
		{
			return 34;
		}
		*/
	}
	
	// 35°C to 40°C range:
	else if (resist <= 6895.4 && resist > 5770.3)
	{
		return 35 + (6895.4 - resist)/225.02;
		/*
		if (resist <= 6895.4 && resist > 6670.38) // 6895.4-5770.3= 1125.1 & 1125.1/5= 225.02
		{
		return 35;
		}
		else if (resist <= 6670.38 && resist > 6445.36)
		{
			return 36;
		}
		else if (resist <= 6445.36 && resist > 6220.34)
		{
			return 37;
		}
		else if (resist <= 6220.34 && resist > 5995.32)
		{
			return 38;
		}
		else if (resist <= 5995.32 && resist > 5770.3)
		{
			return 39;
		}
		*/
	}
	
	// 40°C to 45°C range:
	else if (resist <= 5770.3 && resist > 4852.5)
	{
		return 40 + (5770.3 - resist)/183.56;
		/*
		if (resist <= 5770.3 && resist > 5586.74) //5770.3-4852.5= 917.8 & 917.8/5= 183.56
		{
		return 40;
		}
		else if (resist <= 5586.74 && resist > 5403.18) 
		{
			return 41;
		}
		else if (resist <= 5403.18 && resist > 5219.62)
		{
			return 42;
		}
		else if (resist <= 5219.62 && resist > 5036.06)
		{
			return 43;
		}
		else if (resist <= 5036.06 && resist > 4852.5)
		{
			return 44;
		}
		*/
	}
	
	// 45°C to 50°C range:
	else if (resist <= 4852.5 && resist > 4100)
	{
		return 45 + (4852.5 - resist)/150.5;
		/*
		if (resist <= 4852.5 && resist > 4702) // 4852.5-4100= 752.5 & 752.5/5= 150.5
		{
		return 45;
		}
		else if (resist <= 4702 && resist > 4551.5)
		{
			return 46;
		}
		else if (resist <= 4551.5 && resist > 4401)
		{
			return 47;
		}
		else if (resist <= 4401 && resist > 4250.5)
		{
			return 48;
		}
		else if (resist <= 4250.5 && resist > 4100)
		{
			return 49;
		}
		*/
	}
	
	// 50°C to 55°C range:
	else if (resist <= 4100 && resist > 3479.8)
	{
		return 50 + (4100 - resist)/124.04;
		/*
		if (resist <= 4100 && resist > 3975.96) //4100-3479.8= 620.2 & 620.2/5= 124.04
		{
		return 50;
		}
		else if (resist <= 3975.96 && resist > 3851.92) 
		{
			return 51;
		}
		else if (resist <= 3851.92 && resist > 3727.88)
		{
			return 52;
		}
		else if (resist <= 3727.88 && resist > 3603.84)
		{
			return 53;
		}
		else if (resist <= 3603.84 && resist > 3479.8)
		{
			return 54;
		}
		*/
	}
	
	// 55°C to 60°C range:
	else if (resist <= 3479.8 && resist > 2966.3)
	{
		return 55 + (3479.8 - resist)/102.7;
		/*
		if (resist <= 3479.8 && resist > 3377.1) // 3479.8- 2966.3 = 513.5 & 513.5/5= 102.7 
		{
		return 55;
		}
		else if (resist <= 3377.1 && resist > 3274.4)
		{
			return 56;
		}
		else if (resist <= 3274.4 && resist > 3171.7)
		{
			return 57;
		}
		else if (resist <= 3171.7 && resist > 3069)
		{
			return 58;
		}
		else if (resist <= 3069 && resist > 2966.3)
		{
			return 59;
		}
		*/
	}
	
	// 60°C to 65°C range:
	else if ((resist <= 2966.3) && (resist > 2539.2))
	{
		return 60 + (2966.3 - resist)/85.42;
		/*
		if ((resist <= 2966.3) && (resist > 2880.88)) // 2966.3-2539.2= 427.1 & 427.1/5= 85.42
		{
		return 60;
		}
		else if ((resist <= 2880.88) && (resist > 2795.46))
		{
			return 61;
		}
		else if ((resist <= 2795.46) && (resist > 2710.04))
		{
			return 62;
		}
		else if ((resist <= 2710.04) && (resist > 2624.62))
		{
			return 63;
		}
		else if ((resist <= 2624.62) && (resist > 2539.2))
		{
			return 64;
		}
		*/
	}
	
	// 65°C to 70°C range:
	else if (resist <= 2539.2 && resist > 2182.4)
	{
		return 65 + (2539.2 - resist)/71.36;
		/*
		if (resist <= 2539.2 && resist > 2467.84) //2539.2-2182.4= 356.8 & 356.8/5= 71.36
		{
		return 65;
		}
		else if (resist <= 2467.84 && resist > 2396.48)
		{
			return 66;
		}
		else if (resist <= 2396.48 && resist > 2325.12)
		{
			return 67;
		}
		else if (resist <= 2325.12 && resist > 2253.76)
		{
			return 68;
		}
		else if (resist <= 2253.76 && resist > 2182.4)
		{
			return 69;
		}
		*/
	}
	
	// 70°C to 75°C range:
	else if (resist <= 2182.4 && resist > 1883)
	{
		return 70 + (2182.4 - resist)/59.88;
		/*
		if (resist <= 2182.4 && resist > 2122.52) //2182.4-1883= 299.4 & 299.4/5= 59.88
		{
		return 70;
		}
		else if (resist <= 2122.52 && resist > 2062.64)
		{
			return 71;
		}
		else if (resist <= 2062.64 && resist > 2002.76)
		{
			return 72;
		}
		else if (resist <= 2002.76 && resist > 1942.88)
		{
			return 73;
		}
		else if (resist <= 1942.88 && resist > 1883)
		{
			return 74;
		}
		*/
	}
	
	// 75°C to 80°C range:
	else if (resist <= 1883 && resist > 1630.7)
	{
		return 75 + (1883 - resist)/50.46;
		/*
		if (resist <= 1883 && resist > 1832.54) //1883-1630.7= 252.3 & 252.3/5=50.46
		{
		return 75;
		}
		else if (resist <= 1832.54 && resist > 1782.08)
		{
			return 76;
		}
		else if (resist <= 1782.08 && resist > 1731.62)
		{
			return 77;
		}
		else if (resist <= 1731.62 && resist > 1681.16)
		{
			return 78;
		}
		else if (resist <= 1681.16 && resist > 1630.7)
		{
			return 79;
		}
		*/
	}
	
	// 80°C to 85°C range:
	else if (resist <= 1630.7 && resist > 1417.4)
	{
		return 80 + (1630.7 - resist)/42.66;
		/*
		if (resist <= 1630.7 && resist > 1588.04) //1630.7-1417.4= 213.3 & 213.3/5= 42.66
		{
		return 80;
		}
		else if (resist <= 1588.04 && resist > 1545.38)
		{
			return 81;
		}
		else if (resist <= 1545.38 && resist > 1502.72)
		{
			return 82;
		}
		
		else if (resist <= 1502.72 && resist > 1460.06)
		{
			return 83;
		}
		else if (resist <= 1460.06 && resist > 1417.4)
		{
			return 84;
		}
		*/
	}
	
	// 85°C to 90°C range:
	else if (resist <= 1417.4 && resist > 1236.2)
	{
		return 85 + (1417.4 - resist)/36.24;
		/*
		if (resist <= 1417.4 && resist > 1381.16) //1417.4-1236.2= 181.2 & 181.2/5= 36.24
		{
		return 85;
		}
		else if (resist <= 1381.16 && resist > 1344.92)
		{
			return 86;
		}
		else if (resist <= 1344.92 && resist > 1308.68)
		{
			return 87;
		}
		else if (resist <= 1308.68 && resist > 1272.44)
		{
			return 88;
		}
		else if (resist <= 1272.44 && resist > 1236.2)
		{
			return 89;
		}
		*/
	}
	
	// 90°C to 95°C range:
	else if (resist <= 1236.2 && resist > 1081.8)
	{
		return 90 + (1236.2 - resist)/30.88;
		/*
		if (resist <= 1236.2 && resist > 1205.32) // 1236.2-1081.8= 154.4 & 154.4/5= 30.88
		{
		return 90;
		}
		else if (resist <= 1205.32 && resist > 1174.44)
		{
			return 91;
		}
		else if (resist <= 1174.44 && resist > 1143.56)
		{
			return 92;
		}
		else if (resist <= 1143.56 && resist > 1112.68)
		{
			return 93;
		}
		else if (resist <= 1112.68 && resist > 1081.8)
		{
			return 94;
		}
		*/
	}
	
	// 95°C to 100°C range:
	else if (resist <= 1081.8 && resist > 949.73)
	{
		return 95 + (1081.8 - resist)/26.414;
		/*
		if (resist <= 1081.8 && resist > 1055.39) // 1081.8-949.73= 132.07 & 132.07/5= 26.414
		{
		return 95;
		}
		else if (resist <= 1055.39 && resist > 1028.97)
		{
			return 96;
		}
		else if (resist <= 1028.97 && resist > 1002.56)
		{
			return 97;
		}
		else if (resist <= 1002.56 && resist > 976.144)
		{
			return 98;
		}
		else if (resist <= 976.144 && resist > 949.73)
		{
			return 99;
		}
		*/
	}
	
	/*******************************************************************************************************************
	********************* END PCR RANGE ********************************************************************************
	*******************************************************************************************************************/
	
	// 100°C to 105°C range:
	else if (resist <= 949.73 && resist > 836.4)
	{
		return 100;
	}
	
	// 105°C to 110°C range:
	else if (resist <= 836.4 && resist > 738.81)
	{
		return 105;
	}
	
	// 110°C to 115°C range:
	else if (resist <= 738.81 && resist > 654.5)
	{
		return 110;
	}
	
	// 115°C to 120°C range:
	else if (resist <= 654.5 && resist > 581.44)
	{
		return 115;
	}
	
	// 120°C to 125°C range:
	else if (resist <= 581.44 && resist > 517.94)
	{
		return 120;
	}
	
	// 125°C to 130°C range:
	else if (resist <= 517.94 && resist > 462.59)
	{
		return 125;
	}
	
	// 130°C to 135°C range:
	else if (resist <= 462.59 && resist > 414.2)
	{
		return 130;
	}
	
	// 135°C to 140°C range:
	else if (resist <= 414.2 && resist > 371.79)
	{
		return 135;
	}
	
	// 140°C to 145°C range:
	else if (resist <= 371.79 && resist > 334.51)
	{
		return 140;
	}
	
	// 145°C to 150°C range:
	else if (resist <= 334.51 && resist > 301.66)
	{
		return 145;
	}
	
	// 150°C to 155°C range:
	else if (resist <= 301.66 && resist > 272.64)
	{
		return 150;
	}
	
	//else 
	return 256;
}