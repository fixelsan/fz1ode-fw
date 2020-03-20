/*
 * tmr.c
 *
 *  Created on: March 17, 2020
 *      Author: felix
 */
#include "main.h"
#include "xmc_gpio.h"
#include "tmr.h"
#include "xmc_scu.h"
#include "xmc_ccu4.h"


__WEAK void timer_cb()
{

}

void CCU40_0_IRQHandler()
{
	timer_cb();
}

void tmr_init(uint32_t divider)
{
	XMC_CCU4_SLICE_COMPARE_CONFIG_t slice_config;

	  XMC_CCU4_SLICE_EVENT_CONFIG_t event_config =
	  {
	    .mapped_input = 8,
	    .edge = XMC_CCU4_SLICE_EVENT_EDGE_SENSITIVITY_RISING_EDGE
	  };

	  /* Init CCU4x module */
	  XMC_CCU4_Init(CCU40, XMC_CCU4_SLICE_MCMS_ACTION_TRANSFER_PR_CR);

	  XMC_STRUCT_INIT(slice_config);

	  slice_config.prescaler_initval = divider;
	  XMC_CCU4_SLICE_CompareInit(CCU40_CC40, &slice_config);

	  slice_config.timer_concatenation = true;
	  XMC_CCU4_SLICE_CompareInit(CCU40_CC41, &slice_config);

	  XMC_CCU4_SLICE_SetTimerPeriodMatch(CCU40_CC40, 0xffffU);
	  XMC_CCU4_SLICE_SetTimerPeriodMatch(CCU40_CC41, 0xffffU);

	  // Configure event to start synchronously the two slices
	  XMC_CCU4_SLICE_ConfigureEvent(CCU40_CC40, XMC_CCU4_SLICE_EVENT_0, &event_config);
	  XMC_CCU4_SLICE_ConfigureEvent(CCU40_CC41, XMC_CCU4_SLICE_EVENT_0, &event_config);

	  XMC_CCU4_SLICE_StartConfig(CCU40_CC40, XMC_CCU4_SLICE_EVENT_0, XMC_CCU4_SLICE_START_MODE_TIMER_START);
	  XMC_CCU4_SLICE_StartConfig(CCU40_CC41, XMC_CCU4_SLICE_EVENT_0, XMC_CCU4_SLICE_START_MODE_TIMER_START);

	  // Configure event to stop synchronously the two slices
	  event_config.edge = XMC_CCU4_SLICE_EVENT_EDGE_SENSITIVITY_FALLING_EDGE;
	  XMC_CCU4_SLICE_ConfigureEvent(CCU40_CC40, XMC_CCU4_SLICE_EVENT_1, &event_config);
	  XMC_CCU4_SLICE_ConfigureEvent(CCU40_CC41, XMC_CCU4_SLICE_EVENT_1, &event_config);

	  XMC_CCU4_SLICE_StopConfig(CCU40_CC40, XMC_CCU4_SLICE_EVENT_1, XMC_CCU4_SLICE_END_MODE_TIMER_STOP);
	  XMC_CCU4_SLICE_StopConfig(CCU40_CC41, XMC_CCU4_SLICE_EVENT_1, XMC_CCU4_SLICE_END_MODE_TIMER_STOP);

	  // Assign interrupt node and set priority
	  XMC_CCU4_SLICE_SetInterruptNode(CCU40_CC41, XMC_CCU4_SLICE_IRQ_ID_COMPARE_MATCH_UP, XMC_CCU4_SLICE_SR_ID_0);

	  NVIC_SetPriority(CCU40_0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 16, 0));
	  NVIC_EnableIRQ(CCU40_0_IRQn);

	  // Enable clock to both slices
	  XMC_CCU4_EnableClock(CCU40, (uint8_t)0);
	  XMC_CCU4_EnableClock(CCU40, (uint8_t)1);
}

void tmr_start(uint32_t ticks)
{
	 XMC_CCU4_SLICE_SetTimerCompareMatch(CCU40_CC40, ticks & 0xffffU);
	  XMC_CCU4_SLICE_SetTimerCompareMatch(CCU40_CC41, ticks >> 16U);

	  XMC_CCU4_EnableShadowTransfer(CCU40, XMC_CCU4_SHADOW_TRANSFER_SLICE_0 | XMC_CCU4_SHADOW_TRANSFER_SLICE_1);

	  XMC_CCU4_SLICE_EnableEvent(CCU40_CC41, XMC_CCU4_SLICE_IRQ_ID_COMPARE_MATCH_UP);

	  /* Synchronous start of CCU4x timer slices */
	  XMC_SCU_SetCcuTriggerHigh(XMC_SCU_CCU_TRIGGER_CCU40);
}

void tmr_stop(void)
{
	XMC_CCU4_SLICE_DisableEvent(CCU40_CC41, XMC_CCU4_SLICE_IRQ_ID_COMPARE_MATCH_UP);

	  /* Synchronous stop of CCU4x timer slices */
	  XMC_SCU_SetCcuTriggerLow(XMC_SCU_CCU_TRIGGER_CCU40);
}
void tmr_clear(void)
{
	 XMC_CCU4_SLICE_ClearTimer(CCU40_CC40);
	  XMC_CCU4_SLICE_ClearTimer(CCU40_CC41);
}
uint32_t tmr_gettime(void)
{
	uint32_t timeh = XMC_CCU4_SLICE_GetTimerValue(CCU40_CC41);
	uint32_t timel = XMC_CCU4_SLICE_GetTimerValue(CCU40_CC40);
	while(timeh!=XMC_CCU4_SLICE_GetTimerValue(CCU40_CC41))
	{
		timeh = XMC_CCU4_SLICE_GetTimerValue(CCU40_CC41);
		timel = XMC_CCU4_SLICE_GetTimerValue(CCU40_CC40);
	}

	return (timeh<<16)|(timel);
}
