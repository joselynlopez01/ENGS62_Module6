#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "led.h"
#include "gic.h"		/* interrupt controller interface */
#include "io.h"
#include "xttcps.h"
#include "ttc.h"
#include "xtmrctr.h"
#include "servo.h"
#include "adc.h"
#include <stdbool.h>

#define LOW 4.25 //open
#define HIGH 8.75 // closed


#define TRAFFIC 0
#define PEDESTRIAN 1
#define TRAIN 2
#define MAINTENANCE 3

#define RED 4
#define GREEN 2
#define YELLOW 6
#define BLUE 1
#define RGBLED 6
#define YELLOW2GREEN 7
#define YELLOW2RED 8


static int curr_light = RED;
static int light_counter = 0;
static int traffic_counter = 0;
static int pedes_counter = 0;
static int wait_counter = 0;

static bool pedes_request = false;
static bool blue_light =  false;
static bool train_cleared = false;
static bool train_coming = false;

static bool gate_open;

float dutycycle = 0;
float voltage = 0;

static int state;

static void change_state(int local_state){
	switch(local_state){
		case TRAFFIC:
			traffic_counter = 0;
			state = TRAFFIC;
			break;
		case PEDESTRIAN:
			state = PEDESTRIAN;
			break;
		case TRAIN:
			led_set(RGBLED, true, RED);
			state = TRAIN;
			break;
		case MAINTENANCE:
			printf("Entering MAINTENANCE MODE\n\r");
			fflush(stdout);
			led_set(ALL, false, 0);
			state = MAINTENANCE;
			break;
	}

	printf("CURRENT STATE: %d\n\r", state);
	fflush(stdout);
}

void btn_callback(u32 btn_num){
	if (btn_num == 0 || btn_num == 1){
		pedes_request = true;
	}
}

void sw_callback(u32 sw_num){
	if (sw_num == 0){
		if (state == MAINTENANCE){
			printf("Exiting MAINTENANCE MODE\n\r");
			fflush(stdout);
			printf("train_coming: %d, train_cleared: %d\n\r", train_coming, train_cleared);
			fflush(stdout);
			if(train_cleared == false && train_coming == false){
				servo_set(LOW);
				gate_open = true;
				printf("GATE STATUS: Open\n\r");
				fflush(stdout);
				change_state(TRAFFIC);
			} else if (train_cleared == true){
				led_set(RGBLED, false, 0);
				blue_light = false;
				led_set(ALL, true, 0);
				servo_set(HIGH);
				gate_open = false;
				change_state(TRAIN);
			}
		} else {
			change_state(MAINTENANCE);
		}
	} else if (sw_num == 1){
		if (state == MAINTENANCE){
			//printf("Train coming: %d, Train cleared: %d \n\r ", train_coming, train_cleared);
			if ((train_coming == false) && (train_cleared == false)){
				printf("Train Arriving\n\r");
				fflush(stdout);
				train_coming = true;
				train_cleared = false;
				change_state(MAINTENANCE);
			} else if ((train_coming == true) && (train_cleared == false)){
				printf("Train Clear\n\r");
				fflush(stdout);
				// Wait the 10 seconds to protect humans I guess
				train_cleared = true;
				train_coming = false;
			}
		} else if (train_coming == true && train_cleared == false){
			printf("Train Clear\n\r");
			fflush(stdout);
			// Wait the 10 seconds to protect humans I guess
			train_cleared = true;
			train_coming =  false;
		} else {
			printf("Train Arriving\n\r");
			train_coming = true;
			fflush(stdout);
			servo_set(HIGH);
			gate_open = false;
			printf("GATE STATUS: Closed\n\r");
			fflush(stdout);
			led_set(ALL, true, 0);
			led_set(RGBLED, false, 0);
			train_cleared = false;
			change_state(TRAIN);
		}
	}
}

void ttc_callback(void){
	switch(state){
		case TRAFFIC:
			traffic_counter++;
			if(light_counter == 3){
				if (curr_light == RED){
					led_set(RGBLED, true, YELLOW);
					curr_light = YELLOW2GREEN;
				} else if (curr_light == YELLOW2GREEN){
					led_set(RGBLED, true, GREEN);
					curr_light = GREEN;
				} else if (curr_light == GREEN){
					led_set(RGBLED, true, YELLOW);
					curr_light = YELLOW2RED;
				} else if (curr_light == YELLOW2RED){
					led_set(RGBLED, true, RED);
					curr_light = RED;
				}
				light_counter = 0;
			} else light_counter++;

			if (pedes_request == true && traffic_counter >= 10 && curr_light == RED){
				led_set(ALL, true, 0);
				change_state(PEDESTRIAN);
				pedes_request = false;
			}

			break;
		case PEDESTRIAN:
			if (pedes_counter == 10){
				led_set(ALL, false, 0);
				pedes_counter = 0;
				change_state(TRAFFIC);
			} else pedes_counter++;
			break;
		case TRAIN:
			if (train_cleared == true){
				if (wait_counter == 10){
					wait_counter = 0;
					train_cleared = false;
					servo_set(LOW);
					gate_open = true;
					printf("GATE STATUS: Open\n\r");
					fflush(stdout);
					led_set(ALL, false, 0);
					change_state(TRAFFIC);
				} else wait_counter++;
			}

			break;
		case MAINTENANCE:
			if (blue_light == false){
				led_set(RGBLED, true, BLUE);
				blue_light = true;
			} else {
				led_set(RGBLED, false, 0);
				blue_light = false;
			}

			voltage = adc_get_pot();
			dutycycle = (voltage * 4.5) + 4.25;

			if (dutycycle <= (LOW + 0.01) && (gate_open != true)){
				servo_set(LOW);
				gate_open = true;
				printf("GATE STATUS: Open\n\r");
				fflush(stdout);
			} else if (dutycycle >= HIGH && (gate_open != false)){
				servo_set(HIGH);
				gate_open = false;
				printf("GATE STATUS: Closed\n\r");
				fflush(stdout);
			}

			break;
	}



}

int main()
{
    init_platform();
	led_init();

	if (gic_init() == XST_SUCCESS){
		io_btn_init(btn_callback);
		io_sw_init(sw_callback);
		servo_init();
		adc_init();
		u32 freq = 1;
		ttc_init(freq, ttc_callback);
	}  else {
		printf("GIC cannot connect");
		fflush(stdout);
		return 3;
	}
	printf("Hello\n\r");
	fflush(stdout);


	ttc_start();
	change_state(TRAFFIC);
	servo_set(LOW); // Open traffic
	gate_open = true;

	while (1){
	}
	ttc_stop();
	ttc_close();

    cleanup_platform();
    return 0;
}
