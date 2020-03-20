/*
 * main.c
 *
 *  Created on: March 17, 2020
 *      Author: felix
 */

#include "main.h"
#include "uart_console.h"
#include "xmc_flash.h"
#include "flasher.h"

extern uint32_t __MyImagePoint;

static void hd(const uint8_t* data, size_t length)
{
    for (size_t i = 0; i < length; i += 16) {
    	uartc_puth(i);
    	uartc_putc(' ');
        for (int part = 0, o; part < 2; ++part) {
            for (o = 0; o < 16 && i + o < length; ++o) {
                ((void (*[])(uint8_t)) {
                    ({void __fn__(uint8_t b) { uartc_putb(b); uartc_putc(' ');} __fn__; }),
                            ({void __fn__(uint8_t b) { uartc_putc((b > 32 && b < 128) ? b : '.');} __fn__; }),
                })[part](data[i + o]);
            }
            for (; part == 0 && o < 17; ++o, uartc_writes("   "));
        }
        uartc_writes("\r\n");
    }
}

void timer_cb()
{
	flasher_tmr10();
	XMC_GPIO_ToggleOutput(XMC_GPIO_PORT5,9);
	tmr_stop();
	tmr_clear();
	tmr_start(10*PERIOD_MS);
}

int main()
{


	uartc_init();

	//__enable_irq();
	//XMC_GPIO_SetMode(XMC_GPIO_PORT5,9,XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
	//XMC_GPIO_SetOutputHigh(XMC_GPIO_PORT5,9);
#if 0
	uartc_writes("Flasher  st=");
	uartc_puth(XMC_FLASH_GetStatus());
	uartc_writes(" @");
	uartc_puth(__MyImagePoint);
	uartc_writes("\r\n");

	hd(g_chipid,sizeof(g_chipid));

	if(XMC_FLASH_GetStatus()&XMC_FLASH_STATUS_READ_PROTECTION_INSTALLED)
	{
		uartc_writes("Read protected\r\n");
		res1=XMC_FLASH_VerifyReadProtection(0xdeadbeef,0xcafecafe);
		if(!res1)
		{
			uartc_writes("unp1 passed\r\n");
		}else
			uartc_writes("unp1 failed\r\n");

		uartc_writes("st=");
		uartc_puth(XMC_FLASH_GetStatus());
		uartc_writes("\r\n");
		XMC_FLASH_ClearStatus();
		uartc_writes("st=");
		uartc_puth(XMC_FLASH_GetStatus());
		uartc_writes("\r\n");
		res1=XMC_FLASH_VerifyReadProtection(0xdeadbeef,0xcafecafe);
		if(!res1)
		{
			uartc_writes("unp2 failed\r\n");
		}else
			uartc_writes("unp2 passed\r\n");

		uartc_writes("st=");
		uartc_puth(XMC_FLASH_GetStatus());
		uartc_writes("\r\n");

		XMC_FLASH_EraseUCB(XMC_FLASH_UCB0);
		uartc_writes("UCB Cleared\r\n");

	}else
	{
		XMC_FLASH_EraseUCB(XMC_FLASH_UCB0);
		XMC_FLASH_InstallProtection(0,XMC_FLASH_PROTECTION_READ_GLOBAL,0xdeadbeef,0xcafecafe);
		XMC_FLASH_ConfirmProtection(0);
		uartc_writes("Installed protection\r\n");
	}
#endif
	flasher_init();
	//tmr_init(0);
	//tmr_start(10*PERIOD_MS);

	flasher_rx(0x00);
	flasher_rx(0xaa);
	for(;;)
	{
		flasher_rx(uartc_getc());
	}
}

