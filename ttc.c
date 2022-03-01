#include <stdio.h>
#include "xttcps.h"
#include "xparameters.h"  	/* constants used by the hardware */
#include "xil_types.h"		/* types used by xilinx */
#include "gic.h"		/* interrupt controller interface */

static XTtcPs ttcPs;
static void (*saved_ttc_callback)();
/*
 * ttc_init -- initialize the ttc freqency and callback
 */
void ttc_handler(void *devicep) {
	XTtcPs *dev = (XTtcPs*)devicep;
	saved_ttc_callback();
	XTtcPs_ClearInterruptStatus(dev, XTTCPS_IXR_INTERVAL_MASK);
}

void ttc_init(u32 freq, void (*ttc_callback)(void)){
	saved_ttc_callback = ttc_callback;
	XTtcPs_Config * ConfigPtr;

	ConfigPtr = XTtcPs_LookupConfig(XPAR_XTTCPS_0_DEVICE_ID);
	u32 outcome = XTtcPs_CfgInitialize(&ttcPs, ConfigPtr, ConfigPtr->BaseAddress);
	if (outcome == 0){
		XInterval interval;
		u8 prescaler;
		XTtcPs_CalcIntervalFromFreq(&ttcPs, freq, &interval, &prescaler);
		XTtcPs_SetPrescaler(&ttcPs, prescaler);
		XTtcPs_SetInterval(&ttcPs, interval);
		s32 outcome1 = XTtcPs_SetOptions(&ttcPs, XTTCPS_OPTION_INTERVAL_MODE);
		if (outcome1 == 0){
			XTtcPs_DisableInterrupts(&ttcPs, XTTCPS_IXR_INTERVAL_MASK);
			gic_connect(XPAR_XTTCPS_0_INTR, ttc_handler, &ttcPs);
		}
	}
}

/*
 * ttc_start -- start the ttc
 */
void ttc_start(void){
	XTtcPs_EnableInterrupts(&ttcPs, XTTCPS_IXR_INTERVAL_MASK);
	XTtcPs_Start(&ttcPs);
}

/*
 * ttc_stop -- stop the ttc
 */
void ttc_stop(void){
	XTtcPs_Stop(&ttcPs);
	XTtcPs_DisableInterrupts(&ttcPs, XTTCPS_IXR_INTERVAL_MASK);
}

/*
 * ttc_close -- close down the ttc
 */
void ttc_close(void){
	gic_disconnect(XPAR_XTTCPS_0_INTR);
}
