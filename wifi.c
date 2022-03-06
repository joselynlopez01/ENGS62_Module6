#include "wifi.h"
#include "servo.h"

static XUartPs uart1;
static XUartPs uart0;

static bool train_state;
static bool maintenance_state;

static int STATE;
static u8 long_buffer[200];
static int buffer_counter = 0;
static int numBytes_requested = 1;
static u8 *buffer2;
static void (*saved_callback)(void *buffer);

typedef struct ping{
	int type; // assigned to ping
	int id; // assigned to our id on the class roaster
} ping_t;

typedef struct update_request{
	int type; // must be assigned to UPDATE
	int id; // must be assigned to your id
	int value; // must be assigned to some value
} update_request_t;

typedef struct update_response{
	int type;
	int id;
	int average;
	int values[30];
} update_response_t;

void send_ping(){
	ping_t p;
	p.type = PING;
	p.id = ID;

	XUartPs_Send(&uart0, (u8*) &p, sizeof(ping_t));
}

void send_update(int value){
	update_request_t u;
	u.type = UPDATE;
	u.id = ID;
	u.value = value;

	XUartPs_Send(&uart0, (u8*) &u, sizeof(update_request_t));
}

bool get_train_update(void){
	return (train_state);
}

bool get_maintenance_update(void){
	return (maintenance_state);
}

void set_state(int s){
	STATE = s;
	buffer2 = long_buffer;
}


// uart 0 handler function
static void uart0_handler(void *callBackRef, u32 event, unsigned int eventData){ // Wi-fly
	if(event == XUARTPS_EVENT_RECV_DATA){
		u8 buffer;
		u32 numBytes_requested = 1;



		XUartPs *uart0 = (XUartPs*)callBackRef;

		XUartPs_Recv(uart0, &buffer, numBytes_requested);
		if (STATE == CONFIGURE){
			XUartPs_Send(&uart1, &buffer, numBytes_requested);
		} else if (STATE == PING){
			//XUartPs_Recv(uart0, &buffer, numBytes_requested);
			long_buffer[buffer_counter] = buffer;
			buffer_counter += 1;
			if (buffer_counter == sizeof(ping_t)){
				ping_t* p = (ping_t *) long_buffer;
				buffer_counter = 0;
				printf("[PING %d]\n\r", p->id);
			}
		} else if (STATE == UPDATE){
			long_buffer[buffer_counter] = buffer;
			buffer_counter += 1;
			if (buffer_counter == sizeof(update_response_t)){
				update_response_t* u = (update_response_t *) long_buffer;
				buffer_counter = 0;
				if ((u->values[12] == 1) && (train_state == false)){
					train_state = true;
				} else if ((u->values[12] == 0) && (train_state == true)){
					train_state = false;
				}

				if ((u->values[17] == 1) && (maintenance_state == false)){
					maintenance_state = true;
				} else if ((u->values[17] == 0) && (maintenance_state == true)){
					maintenance_state = false;
				}
			}
		}
	}
}


/*
 * uart1 handler
 */
static void uart1_handler(void *callBackRef, u32 event, unsigned int eventData){ //Terminal, This is done
	XUartPs * uart1 = (XUartPs *) callBackRef;

	if(event == XUARTPS_EVENT_RECV_DATA){

		XUartPs_Recv(uart1, buffer2, numBytes_requested);
		if (STATE == CONFIGURE){
			XUartPs_Send(&uart0, buffer2, numBytes_requested);
			if (*buffer2 == (u8) '\r'){
				*buffer2 = (u8) '\n';
				XUartPs_Send(uart1, buffer2, numBytes_requested);
			}
		} else if (STATE == READ){
			XUartPs_Send(uart1, buffer2, numBytes_requested);
			if (*buffer2 == (u8) '\r'){
				u8 newline = (u8) '\n';
				XUartPs_Send(uart1, &newline, numBytes_requested);
				*buffer2 = '\0';
				saved_callback((void*)long_buffer);
			}else{
				buffer2++;
			}
		}
	}
}


/*
 * initialize the uart for the wifi module
 */
int wifi_init(void (*callback)(void *buffer)){
	saved_callback = callback;
	XUartPs_Config * ConfigPtr0;
	ConfigPtr0 = XUartPs_LookupConfig(XPAR_PS7_UART_0_DEVICE_ID);

	if (XUartPs_CfgInitialize(&uart0,  ConfigPtr0, ConfigPtr0->BaseAddress) == XST_SUCCESS){
		u8 triggerLevel = 1;
		XUartPs_SetFifoThreshold(&uart0, triggerLevel);
		u32 baudRate = 9600;
		if (XUartPs_SetBaudRate(&uart0, baudRate) == XST_SUCCESS){
			XUartPs_SetInterruptMask(&uart0, XUARTPS_IXR_RXOVR);
			XUartPs_SetHandler(&uart0, (XUartPs_Handler) uart0_handler, &uart0);
			gic_connect(XPAR_XUARTPS_0_INTR, (Xil_InterruptHandler) XUartPs_InterruptHandler, &uart0);
		}
	} else {
		printf(" Initialization of UART was not completed");
		fflush(stdout);
		return 1;
	}

	XUartPs_Config * ConfigPtr1;
	ConfigPtr1 = XUartPs_LookupConfig(XPAR_PS7_UART_1_DEVICE_ID);
	if (XUartPs_CfgInitialize(&uart1,  ConfigPtr1, ConfigPtr1->BaseAddress) == XST_SUCCESS){

		u8 triggerLevel = 1;
		XUartPs_SetFifoThreshold(&uart1, triggerLevel);


		XUartPs_SetInterruptMask(&uart1, XUARTPS_IXR_RXOVR);
		XUartPs_SetHandler(&uart1, (XUartPs_Handler) uart1_handler, &uart1);
		gic_connect(XPAR_XUARTPS_1_INTR, (Xil_InterruptHandler) XUartPs_InterruptHandler, &uart1);
	} else {
		printf(" Initialization of UART was not completed");
		fflush(stdout);
		return 1;
	}

	return 0;
}

/*
 * close the wifi
 */
void wifi_close(void){
	XUartPs_DisableUart(&uart0);
	XUartPs_DisableUart(&uart1);
	gic_disconnect(XPAR_XUARTPS_0_INTR);
	gic_disconnect(XPAR_XUARTPS_1_INTR);
}
