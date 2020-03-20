/*
 * uart_console.h
 *
 *  Created on: March 17, 2020
 *      Author: felix lazarev
 */

#ifndef SRC_UART_CONSOLE_H_
#define SRC_UART_CONSOLE_H_

void uartc_init();
int uartc_write(const char *data, int sz);
void uartc_putc(const char dt);
void uartc_puth(unsigned int val);
void uartc_putb(unsigned char val);
void uartc_writes(const char * data);
unsigned char uartc_getc();

#endif /* SRC_UART_CONSOLE_H_ */
