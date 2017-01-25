#ifndef _H_C_included_h_
#define _H_C_included_h_

#include <stdint.h>

void h_c_init(void);
void holdAtTemp(uint16_t , uint16_t);
void ValidateData(void);
uint8_t userInput(void);
void set_PCR(void);
void runPCR(void);
#endif