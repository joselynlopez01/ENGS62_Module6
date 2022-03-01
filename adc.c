/*
 * adc.h -- The ADC module interface
 */
#include <stdio.h>
#include "xadcps.h"
#include "xparameters.h"  	/* constants used by the hardware */
#include "xil_types.h"		/* types used by xilinx */
#include "xadcps.h"

static XAdcPs xadc;

/*
 * initialize the adc module
 */
void adc_init(void){
	XAdcPs_Config * ConfigPtr;

	ConfigPtr = XAdcPs_LookupConfig(XPAR_XADCPS_0_DEVICE_ID);
	int outcome = XAdcPs_CfgInitialize(&xadc, ConfigPtr, ConfigPtr->BaseAddress);
	if (outcome == XST_SUCCESS){
		XAdcPs_SetSequencerMode(&xadc, XADCPS_SEQ_MODE_SAFE);
		XAdcPs_SetAlarmEnables(&xadc, 0);

		if (XAdcPs_SetSeqChEnables(&xadc, XADCPS_SEQ_CH_TEMP | XADCPS_SEQ_CH_VCCINT | XADCPS_SEQ_CH_AUX14) == XST_SUCCESS){
			XAdcPs_SetSequencerMode(&xadc, XADCPS_SEQ_MODE_CONTINPASS);
		}
	}
}

/*
 * get the internal temperature in degree's centigrade
 */
float adc_get_temp(void){
	 u16 rawdata = XAdcPs_GetAdcData(&xadc, XADCPS_CH_TEMP);
	 float temperature = XAdcPs_RawToTemperature(rawdata);
	 return(temperature);
}

/*
 * get the internal vcc voltage (should be ~1.0v)
 */
float adc_get_vccint(void){
	 u16 rawdata = XAdcPs_GetAdcData(&xadc, XADCPS_CH_VCCINT);
	 float voltage = XAdcPs_RawToVoltage(rawdata);
	 return(voltage);
}

/*
 * get the **corrected** potentiometer voltage (should be between 0 and 1v)
 */
float adc_get_pot(void){
	 u16 rawdata = XAdcPs_GetAdcData(&xadc, XADCPS_CH_AUX_MAX - 1);
	 float voltage = XAdcPs_RawToVoltage(rawdata);
	 voltage = voltage/2.98;
	 return(voltage);
}

