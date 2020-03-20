/*
 * tmr.h
 *
 *  Created on: March 17, 2020
 *      Author: felix
 */

#ifndef SRC_TMR_H_
#define SRC_TMR_H_

#include <xmc_common.h>

void tmr_init(uint32_t divider);
void tmr_start(uint32_t ticks);
void tmr_stop(void);
void tmr_clear(void);
uint32_t tmr_gettime(void);


#endif /* SRC_TMR_H_ */
