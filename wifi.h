/*
 * wifi.h
 *
 */
#pragma once

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include <unistd.h>
#include <stdbool.h>
#include "xuartps_hw.h"
#include "gic.h"		/* interrupt controller interface */

#define CONFIGURE 0
#define PING 1
#define UPDATE 2
#define READ 4

#define ID 12

typedef struct ping ping_t;
typedef struct update_request update_request_t;
typedef struct update_response update_response_t;

/*
 * initialize the uart for the wifi module
 */
int wifi_init(void (*callback)(void *buffer));

/*
 * close the wifi
 */
void wifi_close(void);


void set_state(int s);

void send_ping (void);
void send_update (int value);
int* receive_update(void);
