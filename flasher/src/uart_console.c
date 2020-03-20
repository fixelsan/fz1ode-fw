/*
 * uart_console.c
 *
 *  Created on: March 17, 2020
 *      Author: felix
 */

#include "main.h"
#include "xmc_uart.h"
#include "xmc_flash.h"

XMC_UART_CH_CONFIG_t ucon_cfg={
		.data_bits = 8,
		.stop_bits = 1,
		.baudrate = 115200
};

void XMC_AssertHandler(const char * const msg, const char * const file, uint32_t line){
	printf("%s at line %u of %s\n",msg,(unsigned int)line,file);
	//srv_cpureset();
	for(;;)
	{

	}
}

void uartc_init(){
	XMC_UART_CH_Init(XMC_UART0_CH0, &ucon_cfg);
	XMC_GPIO_SetMode(XMC_GPIO_PORT1,4,XMC_GPIO_MODE_INPUT_TRISTATE);
	XMC_UART_CH_SetInputSource(XMC_UART0_CH0,XMC_UART_CH_INPUT_RXD,USIC0_C0_DX0_P1_4);
	XMC_UART_CH_Start(XMC_UART0_CH0);
	XMC_GPIO_SetMode(XMC_GPIO_PORT1,5,(XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT2));
}

int uartc_write(const char *data, int sz){
	int i;

	if(!data)
		return 0;

	for(i=0;i<sz;i++)
	{
		XMC_UART_CH_Transmit(XMC_UART0_CH0,data[i]);
	}

	return sz;
}

void uartc_putc(const char dt){
	XMC_UART_CH_Transmit(XMC_UART0_CH0,dt);
}
const char H2A[16]={
		'0','1','2','3','4','5','6','7','8','9',
		'A','B','C','D','E','F'
};
void uartc_puth(unsigned int val){
	XMC_UART_CH_Transmit(XMC_UART0_CH0,H2A[(val>>28)&0xf]);
	XMC_UART_CH_Transmit(XMC_UART0_CH0,H2A[(val>>24)&0xf]);
	XMC_UART_CH_Transmit(XMC_UART0_CH0,H2A[(val>>20)&0xf]);
	XMC_UART_CH_Transmit(XMC_UART0_CH0,H2A[(val>>16)&0xf]);
	XMC_UART_CH_Transmit(XMC_UART0_CH0,H2A[(val>>12)&0xf]);
	XMC_UART_CH_Transmit(XMC_UART0_CH0,H2A[(val>>8)&0xf]);
	XMC_UART_CH_Transmit(XMC_UART0_CH0,H2A[(val>>4)&0xf]);
	XMC_UART_CH_Transmit(XMC_UART0_CH0,H2A[(val>>0)&0xf]);
}
void uartc_putb(unsigned char val)
{
	XMC_UART_CH_Transmit(XMC_UART0_CH0,H2A[(val>>4)&0xf]);
	XMC_UART_CH_Transmit(XMC_UART0_CH0,H2A[(val>>0)&0xf]);
}
void uartc_writes(const char * data){
	int i;
	i=0;
	while(data[i])
	{
		XMC_UART_CH_Transmit(XMC_UART0_CH0,data[i++]);
	}
}

unsigned char uartc_getc(){
	while((XMC_UART_CH_GetStatusFlag(XMC_UART0_CH0)&XMC_UART_CH_STATUS_FLAG_RECEIVE_INDICATION)==0);
	XMC_UART_CH_ClearStatusFlag(XMC_UART0_CH0,XMC_UART_CH_STATUS_FLAG_RECEIVE_INDICATION);
	return XMC_UART_CH_GetReceivedData(XMC_UART0_CH0);
}

