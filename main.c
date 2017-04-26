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
* File Name    : main.c
* Version      : 1.0.0
* Device(s)    : R5F104PJ
* Tool-Chain   : CA78K0R
* Description  : This file implements main function.
* Creation Date: 09-Sep-15
***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#pragma interrupt INTIT r_it_interrupt
#include "r_macro.h"
#include "r_spi_if.h"
#include "lcd.h"
#include "uart.h"
#include "adc.h"
#include "led.h"
#include "api.h"
#include <string.h>
#include "stdio.h"
#include "switch.h"
/***********************************************************************************************************************
Funtion declaration
***********************************************************************************************************************/
void r_main_userinit(void);
/**********************************************************************888
extern variable declare
******************************************************************/

volatile int counter = 0; //timer

extern float final_voltage; //acd
//extern unsigned char LCD_Line; 
extern int numb;
extern char temp_receive[UART_RX_BUFFER_LEN];

extern unsigned int state; //sw
extern unsigned int enable_switch;//sw
int error_flag = NO_ERROR;//uart
int index=0;//uart
int lcd_status;//uart

int time_flag;//timer interrupt
/***********************************************************************************************************************
* Function Name: main
* Description  : This function implements main function.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void main(void)
{
	int i;
	Uart_Init();//uart int
	Uart_Start();//Start UART1 communication
	
	R_IT_Create();//timer init
	R_IT_Start(); // start timer
	
	ADC_Create();//adc init
	//ADC_Set_OperationOn();
	ADC_Start(); //Start an A/D conversion
	
	ledinit();//led init
	
	r_main_userinit();//initialize lcd
	InitialiseLCD();//initialize lcd
	ClearLCD();
	
	
	//enable_switch = 1;//enable check switch
	while (1U)
	{   
	  	/*Get switch when error occur*/
		state = getswitch();
/////////////////////////////////////////////////////////////
		/*When switch was pressed , reset LCD */
		if (state==0x50 && error_flag != NO_ERROR)
		{
			ClearLCD();//clear lcd
			error_flag = NO_ERROR;//err flag
			SIR03 = 0x07;//Reset register check error of uart_error
			P1_bit.no0 =0;//light 2 off
			//rest buffer
			for (i=0; i<9 ;i++)
			{
				Uart_ClearBuff(receive_data[i], UART_RX_BUFFER_LEN - 1);
			}
			//reset temp buffer for display
			Uart_ClearBuff(&temp_receive[0], UART_RX_BUFFER_LEN - 1);			
			led_status_1.status_led =0x0000;//reset buff led
			//reset led
			P6 = 0xff;
			P4 = 0xff;
			P10_bit.no1 = 1;
			P15_bit.no2 = 1;
			numb =0;//Reset index variable
		}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
		/*Check error uart*/
		check_error_uart();
		if(error_flag == MSG_ERROR)
		{
			display_msg_error();
		}
		/*if no error*/		
		if(error_flag == NO_ERROR)    
		{    
		   	/* Check UART1 receive status */
		        if(status == UART_RECEIVE_DONE)
		        {
				/*If buffer emty*/
				if(lcd_status ==0)
				{
				        status = 0;	
					/* Replace the last element by NULL */
				        receive_data[numb][UART_RX_BUFFER_LEN - 1] = '\0';
					/*Display data to board*/
					display_data();
				}
				/*Reset status of buffer after display data*/
				else if (lcd_status ==1)
				{
					lcd_status =0;
				}
				
			}
		}
//////////////////////////////////////////////////////////////////////////////////////////		
		/*Check timer after 200ms*/
		if (time_flag ==1)
		{
			/*Start UART */
			Uart_Start();
			/*send led status*/
			Send_led();
			/* Start an A/D conversion */
			ADC_Start();
			/*Calculate Voltage of VR1*/
			ADC_calculate();
			/*send adc value*/
			Send_ADC();		    
			/*reset timer*/
			counter =0;	   
			time_flag =0;
		} 
	}
}



/***********************************************************************************************************************
* Function Name: r_main_userinit
* Description  : This function reset LCD setting
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void r_main_userinit(void)
{
    int i =0;
     /* Enable interrupt */
    EI();
    
   
    /* Output a logic LOW level to external reset pin*/
    P13_bit.no0 = 0;
    for (i = 0; i < 1000; i++)
    {
        NOP();
    }

    /* Generate a raising edge by ouput HIGH logic level to external reset pin */
    P13_bit.no0 = 1;
    for (i = 0; i < 1000; i++)
    {
        NOP();
    }

    /* Output a logic LOW level to external reset pin, the reset is completed */
    P13_bit.no0 = 0;
    
     /* Initialize SPI channel used for LCD */
    R_SPI_Init(SPI_LCD_CHANNEL);
	
    /* Initialize Chip-Select pin for LCD-SPI: P145 (Port 14, pin 5) */
    R_SPI_SslInit(
    SPI_SSL_LCD,             /* SPI_SSL_LCD is the index defined in lcd.h */
    (unsigned char *)&P14,   /* Select Port register */
    (unsigned char *)&PM14,  /* Select Port mode register */
    5,                       /* Select pin index in the port */
    0,                       /* Configure CS pin active state, 0 means active LOW level  */
    0                        /* Configure CS pin active mode, 0 means active per transfer */
    );
    
    
}
__interrupt static void r_it_interrupt(void)
{
	if (counter >=20)
	{
		time_flag = 1;
	}
	counter++;
}
/******************************************************************************
End of file
******************************************************************************/
