/*
 * flasher.h
 *
 *  Created on: March 17, 2020
 *      Author: felix
 */

#ifndef SRC_FLASHER_H_
#define SRC_FLASHER_H_

typedef struct{
	unsigned int startaddr;
	unsigned int size;
}sector_table_entry_t;

#define FLASHER_OK			0
#define FLASHER_E_FAILED	-1
#define FLASHER_E_PROTECTED	-2
#define FLASHER_E_IVADDR	-3
#define FLASHER_E_IVSIZE	-4


#define PAGE_START_MSK		0xff
#define SECTOR_START_MSG	0xfff
#define BASE_MASK			(~0x1fffff)

#define PAGE_SIZE			256

#define HEADER_BLK_SIZE		16
#define DATA_BLK_SIZE		(PAGE_SIZE+8)

#define HEADER_BLOCK		0x00
#define DATA_BLOCK			0x01
#define EOT_BLOCK			0x02

#define BSL_PROGRAM_FLASH		0x00
#define BSL_RUN_FROM_FLASH		0x01
#define BSL_RUN_FROM_ICACHE		0x02
#define BSL_ERASE_FLASH			0x03
#define BSL_PROTECT_STATUS		0x04
#define BSL_PROTECT_FLASH		0x05

#define BSL_BLOCK_TYPE_ERR		0xff
#define BSL_MODE_ERR			0xfe
#define BSL_CHKSUM_ERR			0xfd
#define BSL_ADDR_ERR			0xfc
#define BSL_ERASE_ERR			0xfb
#define BSL_PROG_ERR			0xfa
#define BSL_VERFY_ERR			0xf9
#define BSL_PROT_ERR			0xf8
#define BSL_SUCCESS				0x55

void flasher_init();
void flasher_rx(unsigned char rx);
void flasher_tmr10();

#endif /* SRC_FLASHER_H_ */
