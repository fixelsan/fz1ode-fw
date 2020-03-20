/*
 * flasher.c
 *
 *  Created on: March 17, 2020
 *      Author: felix
 */


#include "main.h"
#include "flasher.h"
#include "xmc_flash.h"
#include "xmc_gpio.h"

enum{
	FL_WAIT_CMD=0,
	FL_WAIT_LEN,
	FL_WAIT_ADDR,
	FL_WAIT_DATA,
	FL_EXECUTE,
};

enum{
	WAIT_LEN=0x10000,
	WAIT_ADDR=0x20000,
	WAIT_DATA=0x40000
};

uint32_t state;
uint32_t state_cnt;
uint32_t required;
uint32_t addr;
uint32_t len;
uint8_t cmd;

uint8_t page_buf[PAGE_SIZE];
uint8_t page_rbuf[PAGE_SIZE];

void flasher_handle_cmd();
void flasher_handle_len();
void flasher_handle_addr();
void flasher_handle_data();
void flasher_execute_cmd();

typedef enum{
	CMD_ACK=0,
	CMD_ERASE_SECTOR,	//1
	CMD_WRITE_PAGE,		//2
	CMD_PROTECT,		//3
	CMD_UNPROTECT,		//4
	CMD_VERIFY_PAGE,	//5
}fl_cmds_e;

uint32_t cmd_req[256];


void flasher_init()
{
	memset(cmd_req,0,sizeof(cmd_req));
	state=FL_WAIT_CMD;
	required=0;
	cmd_req[CMD_ACK]=(WAIT_DATA|1); //1 byte rx...
	cmd_req[CMD_ERASE_SECTOR]=WAIT_ADDR;
	cmd_req[CMD_WRITE_PAGE]=(WAIT_ADDR|WAIT_LEN|WAIT_DATA);
	cmd_req[CMD_PROTECT]=(WAIT_DATA|8);
	cmd_req[CMD_UNPROTECT]=(WAIT_DATA|8);
	cmd_req[CMD_VERIFY_PAGE]=(WAIT_ADDR|WAIT_LEN|WAIT_DATA);
}

void flasher_rx(unsigned char rx)
{
	switch(state){
	case FL_WAIT_CMD:
		cmd=rx;
		state_cnt=0;
		flasher_handle_cmd();
		break;
	case FL_WAIT_ADDR:
		switch(state_cnt)
		{
			case 0: addr=rx; state_cnt=1;break;
			case 1: addr|=rx<<8; state_cnt=2; break;
			case 2: addr|=rx<<16; state_cnt=3; break;
			case 3: addr|=rx<<24; state_cnt=0; flasher_handle_addr(); break;
			default : flasher_handle_addr(); break;
		}
		break;
	case FL_WAIT_LEN:
		switch(state_cnt)
		{
			case 0: len=rx; state_cnt=1;break;
			case 1: len|=rx<<8; state_cnt=2; break;
			case 2: len|=rx<<16; state_cnt=3; break;
			case 3: len|=rx<<24; state_cnt=0; flasher_handle_len(); break;
			default : flasher_handle_len(); break;
		}
		break;
	case FL_WAIT_DATA:
		page_buf[state_cnt++]=rx;
		if( (state_cnt>=PAGE_SIZE) || (state_cnt>=len))
		{
			flasher_handle_data();
		}
		break;


	}
}

void flasher_tmr10()
{
	uartc_putc('/');
}

void flasher_handle_cmd()
{
	if(cmd_req[cmd]==0)
	{
		uartc_putc(BSL_MODE_ERR);
		return;
	}
	required=cmd_req[cmd];
	state_cnt=0;
	if(required&0xffff)
		len=required&0xffff;
	if(required&WAIT_ADDR)
	{
		state=FL_WAIT_ADDR;
		return;
	}else if(required&WAIT_LEN)
	{
		state=FL_WAIT_LEN;
		return;
	}else if(required&WAIT_DATA)
	{
		state=FL_WAIT_DATA;
		return;
	}
	flasher_execute_cmd();
}
void flasher_handle_len()
{
	required&=~WAIT_LEN;
	if(required&WAIT_ADDR)
	{
		state=FL_WAIT_ADDR;
		return;
	}if(required&WAIT_DATA)
	{
		state=FL_WAIT_DATA;
		return;
	}
	flasher_execute_cmd();
	state=FL_WAIT_CMD;

}

void flasher_handle_addr()
{
	required&=~WAIT_ADDR;
	if(required&WAIT_LEN)
	{
		state=FL_WAIT_LEN;
		return;
	}if(required&WAIT_DATA)
	{
		state=FL_WAIT_DATA;
		return;
	}
	flasher_execute_cmd();
	state=FL_WAIT_CMD;
}

void flasher_handle_data()
{
	required&=~WAIT_DATA;
	flasher_execute_cmd();
	state=FL_WAIT_CMD;
}

void flasher_execute_cmd()
{
	uint32_t rem;
	uint32_t c1,c2;
	state=FL_WAIT_CMD;

	switch(cmd){
	case CMD_ACK:
		rem=XMC_FLASH_GetStatus() & (uint32_t)XMC_FLASH_STATUS_READ_PROTECTION_INSTALLED;
		uartc_putc(0x55);uartc_putc(page_buf[0]);
		if(rem)
			uartc_putc(1);
		else
			uartc_putc(2);
		break;
	case CMD_ERASE_SECTOR:
		XMC_FLASH_EraseSector((uint32_t *)addr);
		uartc_putc(0x55);
		break;
	case CMD_WRITE_PAGE:
		memset(page_rbuf,0,256);
		if(addr&0xff)
		{
			 rem = 256-(addr&0xff);
			if(len>rem)
			{
				uartc_putc(0xff);
				return;
			}

			memcpy(page_rbuf,addr&0xffffff00,256);
			memcpy(&page_rbuf[addr&0xff],page_buf,len);
			addr&=0xffffff00;

		}else
		{
			memcpy(page_rbuf,page_buf,len);
		}
		XMC_FLASH_ProgramPage((uint32_t*)addr,page_rbuf);
		uartc_putc(0x55);
		break;
	case CMD_PROTECT:
		memcpy(&c1,page_buf,4);
		memcpy(&c2,&page_buf[4],4);
		if ((XMC_FLASH_GetStatus() & (uint32_t)XMC_FLASH_STATUS_READ_PROTECTION_INSTALLED) != 0U)
		{
			uartc_putc(0xff);
			return;
		}
		XMC_FLASH_EraseUCB(XMC_FLASH_UCB0);
		XMC_FLASH_InstallProtection(0,XMC_FLASH_PROTECTION_READ_GLOBAL,c1,c2);
		XMC_FLASH_ConfirmProtection(0);
		uartc_putc(0x55);
		break;
	case CMD_UNPROTECT:
		memcpy(&c1,page_buf,4);
		memcpy(&c2,&page_buf[4],4);
		rem=XMC_FLASH_VerifyReadProtection(c1,c2);
		if(rem)
		{
			XMC_FLASH_EraseUCB(XMC_FLASH_UCB0);
			uartc_putc(0x55);
		}else
		{
			uartc_putc(0xff);
		}
		break;
	}

}
