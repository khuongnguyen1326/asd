/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only 
* intended for use with Renesas products. No other uses are authorized. This 
* software is owned by Renesas Electronics Corporation and is protected under 
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING 
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT 
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
* AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR 
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE 
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software 
* and to discontinue the availability of this software.  By using this software, 
* you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2011, 2013 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : adc.c
* Version      : 1.0.0
* Device(s)    : R5F104PJ
* Tool-Chain   : CA78K0R
* Description  : This file implements device driver for ADC module.
* Creation Date: 11-Sep-15 
***********************************************************************************************************************/

/**********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#pragma interrupt INTAD adc_interrupt

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_macro.h"
#include "iodefine.h"
#include "iodefine_ext.h"
#include "adc.h"
#include "uart.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/


volatile uint8_t Adc_Status;
volatile uint16_t gADC_Result = 0;
float cur_voltage ;
int adc_value;
float final_voltage;
char tx_buff_analog[13]= "$1872,A";
/***********************************************************************************************************************
* Function Name: ADC_Create
* Description  : This function initializes the AD converter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void ADC_Create(void)
{
	ADCEN = 1U;                       /* supply AD clock */
	ADM0 = 0x00;                     /* disable AD conversion and clear ADM0 register */
	ADMK = 1U;                        /* disable INTAD interrupt */
	ADIF = 0U;                        /* clear INTAD interrupt flag */
	/* Set INTAD level2 priority */
	ADPR1 = 1U;
	ADPR0 = 0U;
	/* Set ANI0 - ANI8 pin as analog input */
	PM2 |= 0xFF;
	PM15 |= 0x01;
	ADM0 = 0x00;			/* select clock fclk/64*/
	ADM1 = 0x00;			/* select software trigger*/
	ADM2 = 0x00;			/* select 10bit resolution*/
	ADUL = 0xFF;
	ADLL = 0x00;
	ADS = 0x08;
	ADCE = 1U;                        /* enable AD comparator */
}

/***********************************************************************************************************************
* Function Name:ADC_Start
* Description  : This function starts the AD converter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void ADC_Start(void)
{
	ADIF = 0U;  /* clear INTAD interrupt flag */
	ADMK = 0U;  /* enable INTAD interrupt */
	ADCS = 1U;  /* enable AD conversion */
	Adc_Status = ADC_PENDING;
}

/***********************************************************************************************************************
* Function Name:ADC_Stop
* Description  : This function stops the AD converter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void ADC_Stop(void)
{
	ADCS = 0U;  /* disable AD conversion */
	ADMK = 1U;  /* disable INTAD interrupt */
	ADIF = 0U;  /* clear INTAD interrupt flag */
}

/***********************************************************************************************************************
* Function Name:ADC_Get_Result
* Description  : This function returns the conversion result in the buffer.
* Arguments    : buffer - the address where to write the conversion result
* Return Value : None
***********************************************************************************************************************/
void ADC_Get_Result(uint16_t * const buffer)
{
	*buffer = (uint16_t)(ADCR >> 6U);
}

/***********************************************************************************************************************
* Function Name: adc_interrupt
* Description  : This function is INTAD interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void adc_interrupt(void)
{
	Adc_Status = ADC_DONE;
}
/************************************************************
* Function Name : ADC_calculate
* Description 	: This function is used to calculate  Voltage of VR1
***************************************************/
void ADC_calculate(void)
{
	float _voltage_temp;
	/* Wait for the A/D conversion to complete */
	while(Adc_Status != ADC_DONE);
	/* Clear ADC flag */
	Adc_Status = 0;

	/* Shift the ADC result contained in the 16-bit ADCR register */
	gADC_Result = ADCR >> 6;

	/* Convert ADC result into a character string, and store in the local */
	cur_voltage = (gADC_Result*FULL_RANGE_SCALE)/MAX_LEVEL_ADC;
	
	
	_voltage_temp = final_voltage; 

	/*consider Error of ADC*/
	if (((cur_voltage - _voltage_temp)>OVERALL_ERROR) || ((_voltage_temp - cur_voltage)>OVERALL_ERROR))
	{
	_voltage_temp = cur_voltage;
	}
	final_voltage = _voltage_temp;
	
	adc_value = (int)(((final_voltage)*999)/3.3);
}

/************************************************************
* Function Name : Send_ADC
* Description 	: This function is used to send Voltage of VR1 
***************************************************/
void Send_ADC()
{
	int adc_temp1 ;
	int adc_temp2 ;
	int adc_temp3 ;
	adc_temp1 = (int)(adc_value/100);
	adc_temp2 = (int)((adc_value- adc_temp1*100)/10);
	adc_temp3 = (int) (adc_value - adc_temp1*100 - adc_temp2*10);
	tx_buff_analog[7] = (char)(adc_temp1 + 48);
	tx_buff_analog[8] = (char)(adc_temp2 + 48);
	tx_buff_analog[9] = (char)(adc_temp3 + 48);
	tx_buff_analog[10] = '^';
	Uart_Transmit(&tx_buff_analog[0],13);
}
/******************************************************************************
End of file
******************************************************************************/
