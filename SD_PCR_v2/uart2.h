#ifndef _uart2_included_h_
#define _uart2_included_h_

#include <stdint.h>

void uart_init2(uint32_t baud);
void uart_putchar2(uint8_t c);
uint8_t uart_getchar2(void);
uint8_t uart_available2(void);

#endif
