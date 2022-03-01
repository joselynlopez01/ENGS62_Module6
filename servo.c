/*
 * servo.c
 */

#include <stdio.h>
#include "xtmrctr.h"
#include "xparameters.h"  	/* constants used by the hardware */
#include "xil_types.h"		/* types used by xilinx */

#define TOTAL_OPTIONS XTC_DOWN_COUNT_OPTION | XTC_EXT_COMPARE_OPTION | XTC_PWM_ENABLE_OPTION
//#define LOW 4.25
//#define HIGH 8.75

static XTmrCtr TmrCtr;
static u32 resetValue_0 = 1000000; // Our goal is to find;
static u32 resetValue_1; // Our goal is to find


/*
 * Initialize the servo, setting the duty cycle to 7.5%
 */
void servo_init(void){
	resetValue_1 = 75000; // Our goal is to find

	XTmrCtr_Initialize(&TmrCtr, XPAR_TMRCTR_0_DEVICE_ID);

	// Timer 0 and Timer 1
	XTmrCtr_Stop(&TmrCtr, XTC_TIMER_0); // Stops the timer counter by disabling it
	XTmrCtr_Stop(&TmrCtr, XTC_TIMER_1); // Stops the timer counter by disabling it

	XTmrCtr_SetOptions(&TmrCtr, XTC_TIMER_0, TOTAL_OPTIONS);
	XTmrCtr_SetOptions(&TmrCtr, XTC_TIMER_1, TOTAL_OPTIONS);

	XTmrCtr_SetResetValue(&TmrCtr, XTC_TIMER_0, resetValue_0);
	XTmrCtr_SetResetValue(&TmrCtr, XTC_TIMER_1, resetValue_1);

	XTmrCtr_Start(&TmrCtr, XTC_TIMER_0);
	XTmrCtr_Start(&TmrCtr, XTC_TIMER_1);

}

/*
 * Set the dutycycle of the servo
 */
void servo_set(double dutycycle){

	if ((dutycycle <= 8.78) && (dutycycle >= 4.24)){
		resetValue_1 = (dutycycle/100)* resetValue_0; // Our goal is to find

		XTmrCtr_Stop(&TmrCtr, XTC_TIMER_0);
		XTmrCtr_Stop(&TmrCtr, XTC_TIMER_1);

		XTmrCtr_SetResetValue(&TmrCtr, XTC_TIMER_0, resetValue_0);
		XTmrCtr_SetResetValue(&TmrCtr, XTC_TIMER_1, resetValue_1);

		XTmrCtr_Start(&TmrCtr, XTC_TIMER_0);
		XTmrCtr_Start(&TmrCtr, XTC_TIMER_1);
	}

}
