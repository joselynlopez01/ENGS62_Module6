/*
 * led.h -- led module interface
 *
 */
//#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <xgpio.h>		  	/* axi gpio */
#include <xgpiops.h>		/* processor gpio */
#include "xparameters.h"  	/* constants used by the hardware */
#include "xil_types.h"		/* types used by xilinx */

#include "platform.h"						/* ZYBOboard interface */
#include "led.h"


/* led states */
#define LED_ON true
#define LED_OFF false

#define ALL 0xFFFFFFFF		/* A value designating ALL leds */
#define OUTPUT 0x0							/* setting GPIO direction to output */
#define OUTPUTPs 0x1							/* setting GPIOPs direction to output */
#define CHANNEL1 1							/* channel 1 of the GPIO port */

static XGpio port;
static XGpio port6;
static XGpioPs portPs;

static bool LED4_status = false;
/*
 * Initialize the led module
 */
void led_init(void){
	//init_platform();							/* initialize the hardware platform */
	XGpio_Initialize(&port, XPAR_AXI_GPIO_0_DEVICE_ID);	/* initialize device AXI_GPIO_0 */
	XGpio_SetDataDirection(&port, CHANNEL1, OUTPUT);	    /* set tristate buffer to output */

	XGpioPs_Config * ConfigPtr;
	ConfigPtr = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
	XGpioPs_CfgInitialize(&portPs, ConfigPtr, ConfigPtr->BaseAddr);
	XGpioPs_SetDirectionPin(&portPs, 7, 1);
	XGpioPs_SetOutputEnablePin(&portPs, 7, 1);

	// Port 6
	XGpio_Initialize(&port6, XPAR_AXI_GPIO_3_DEVICE_ID);	/* initialize device AXI_GPIO_0 */
	XGpio_SetDataDirection(&port6, CHANNEL1, OUTPUT);	    /* set tristate buffer to output */
}



/*
 * Set <led> to one of {LED_ON,LED_OFF,...}
 *
 * <led> is either ALL or a number >= 0
 * Does nothing if <led> is invalid
 */


void led_set(u32 led, bool tostate, u32 color){
	u32 reg_value;
	u32 new_value;
	u32 op;
	reg_value = XGpio_DiscreteRead(&port, CHANNEL1);

	if(led == ALL){
		if (tostate == true) {
			new_value = 0xF;
		} else {
			new_value = 0x0;
		}
		XGpio_DiscreteWrite(&port, CHANNEL1, new_value);
		return;
	} else if(led == 0 ) {
		op =  0x1;
	} else if(led == 1) {
		op =  0x2;
	} else if(led == 2) {
		op =  0x4;
	} else if(led == 3){
		op =  0x8;
	}

	if(led != ALL && led >= 0 && led <= 3){
		if (tostate == LED_ON){
			new_value = reg_value | op;
		} else if (tostate == LED_OFF && led_get(led) == LED_OFF){
			new_value = reg_value ^ 0x0;
		} else {
			new_value = reg_value ^ op; //turning if off
		}
		XGpio_DiscreteWrite(&port, CHANNEL1, new_value);
	}

	if(led == 4){
		if (tostate == LED_ON){
			XGpioPs_WritePin(&portPs, 7, 0x1);
		} else {
			XGpioPs_WritePin(&portPs, 7, 0x0);
		}
		LED4_status=!LED4_status;
	}

	if(led == 6){
		XGpio_DiscreteWrite(&port6, CHANNEL1, color);
	}

}



/*
 * Get the status of <led>
 *
 * <led> is a number >= 0
 * returns {LED_ON,LED_OFF,...}; LED_OFF if <led> is invalid
 */
bool led_get(u32 led){
	u32 reg_value;
	u32 comp_value;
	u32 ans;
	reg_value = XGpio_DiscreteRead(&port, CHANNEL1);


	if (led <=3){
		if (led == 0x0){
			comp_value = 0x1;
		} else if (led == 0x1){
			comp_value = 0x2;
		} else if (led == 0x2){
			comp_value = 0x4;
		} else if (led == 0x3){
			comp_value = 0x8;
		}

		ans = reg_value & comp_value;

		if (ans == 0x0){
			return(LED_OFF);
		} else {
			return(LED_ON);
		}

	} else if (led == 4){
		if (LED4_status == LED_OFF){
			return(LED_OFF);
		} else return(LED_ON);
	} else return(LED_OFF);
}



/*
 * Toggle <led>
 *
 * <led> is a value >= 0
 * Does nothing if <led> is invalid
 */
void led_toggle(u32 led){
	if (led_get(led) == 0){
		led_set(led, LED_ON, 0);
	} else {
		led_set(led, LED_OFF, 0);
	}
}

//int main(void){
//	led_init();
//
//	bool answer;
//
//	led_set(3, LED_ON);
//	led_set(2, LED_ON);
//	led_set(0, LED_ON);
//	led_set(1, LED_ON);
//	led_set(2, LED_OFF);
//	led_set(1, LED_OFF);
//	led_set(0, LED_OFF);
//	led_set(3, LED_OFF);
//	led_set(ALL, LED_ON);
//	led_set(ALL, LED_OFF);
//
//	answer = led_get(0);
//	printf("n\rLED0 is %d\n\r", answer);
//	fflush(stdout);
//
//	answer = led_get(1);
//	printf("LED1 is %d\n\r", answer);
//	fflush(stdout);
//
//	answer = led_get(2);
//	printf("LED2 is %d\n\r", answer);
//	fflush(stdout);
//
//	answer = led_get(3);
//	printf("LED3 is %d\n\r", answer);
//	fflush(stdout);
//
//	led_toggle(0);
//	led_toggle(1);
//	led_toggle(2);
//	led_toggle(3);
//	led_toggle(0);
//	led_toggle(1);
//	led_toggle(2);
//	led_toggle(3);
//
//	led_set(4, LED_ON);
//	led_set(4, LED_OFF);
//
//	led_set(6, LED_ON);
//	printf("Done\n\r");
//	fflush(stdout);
//
//	cleanup_platform();					/* cleanup the hardware platform */
//	return 0;
//}
//
