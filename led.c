
 /******************************************************************************
* DISCLAIMER

* This software is supplied by Renesas Electronics Corporation and is only 
* intended for use with Renesas products. No other uses are authorized.

* This software is owned by Renesas Electronics Corporation and is protected under 
* all applicable laws, including copyright laws.

* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES 
* REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, 
* INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
* PARTICULAR PURPOSE AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY 
* DISCLAIMED.

* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES 
* FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS 
* AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

* Renesas reserves the right, without notice, to make changes to this 
* software and to discontinue the availability of this software.  
* By using this software, you agree to the additional terms and 
* conditions found by accessing the following link:
* http://www.renesas.com/disclaimer
******************************************************************************/
/* Copyright (C) 2015 Renesas Electronics Corporation. All rights reserved.  */
/******************************************************************************	
* File Name    : led.c
* Version      : 1.00
* Description  : 
******************************************************************************
* History : DD.MM.YYYY Version Description
*         : 07.04.2016 1.00    First Release
******************************************************************************/
#include "r_macro.h"
#include "led.h"
#include "math.h"
#include "r_spi_if.h" /* SPI driver interface */
#include "lcd.h"      /* LCD driver interface */
#include "stdio.h"
#include "uart.h"
#include "stdlib.h"

//#define OFFSET_LEDNUMB	3
unsigned char *PORT[13] ={PORT6,PORT4,PORT6,PORT4,PORT6,PORT4,PORT6,PORT4,PORT6,PORT15,PORT6,PORT10,PORT4};
unsigned char _value_on[13] ={LED3_ON,LED4_ON,LED5_ON,LED6_ON,LED7_ON,LED8_ON,LED9_ON,LED10_ON,LED11_ON,LED12_ON,LED13_ON,LED14_ON,LED15_ON};
unsigned char _value_off[13]={LED3_OFF,LED4_OFF,LED5_OFF,LED6_OFF,LED7_OFF,LED8_OFF,LED9_OFF,LED10_OFF,LED11_OFF,LED12_OFF,LED13_OFF,LED14_OFF,LED15_OFF};

char tx_buff_led[13] = "$1872,L";
led_status led_status_1;
extern int error_flag;
extern numb;
/*******************************************************************************************
Function name :ledinit
Decription    :This function setting port mode of 12 led 
******************************************************************************************/
void ledinit()
{
	ADPC =1;
	PM4 &=0xC1;
	PM6 &=0x03;
	PM15 &=0xfb;
	PM10 &=0xfd;
	P6 =0xff;
	P4 =0xff;
	P15 =0xff;
	P10 =0xff;
	PM1_bit.no0 = 0;
}
/**********************************************************************************
Function name :status_led
Decription    :This function asign led status value
***************************************************************************************/
void status_led()
{
	led_status_1.BIT.bit0 = ~P6_bit.no2; /*led 3*/
	led_status_1.BIT.bit1 = ~P4_bit.no2; /*led 4*/
	led_status_1.BIT.bit2 = ~P6_bit.no3; /*led 5*/
	led_status_1.BIT.bit3 = ~P4_bit.no3; /*led 6*/
	
	led_status_1.BIT.bit4 = ~P6_bit.no4; /*led 7*/
	led_status_1.BIT.bit5 = ~P4_bit.no4; /*led 8*/
	led_status_1.BIT.bit6 = ~P6_bit.no5; /*led 9*/
	led_status_1.BIT.bit7 = ~P4_bit.no5; /*led 10*/

	led_status_1.BIT.bit8 = ~P6_bit.no6; /*led 11*/
	led_status_1.BIT.bit9 = ~P15_bit.no2; /*led 12*/
	led_status_1.BIT.bit10 = ~P6_bit.no7; /*led 13*/
	led_status_1.BIT.bit11 = ~P10_bit.no1; /*led 14*/
	
	led_status_1.BIT.bit12 = ~P4_bit.no1; /*led 15*/
	led_status_1.BIT.bit13 = 0; 
	led_status_1.BIT.bit14 = 0; 
	led_status_1.BIT.bit15 = 0; 
}
/***********************************************************************************
Function name :check_led_status
Decription    :This function check valid of data received to turn on -off led
Paremeter     :Data received
*********************************************************************************/
void check_led_status(char *buff)
{
	
	int led_number =0;
	int i =0;
	char temp[3] ={'\0','\0','\0'};
	numb=0;
	if (buff[1] =='L')
	{
		/*take number of led*/
		for (i = 3 ; buff[i] != ','; i++)
		{
			temp[i-3] = buff[i];
		}
		led_number = atoi(temp);
		led_number = led_number -3;
		/*check valid of led number*/
		if (led_number <0 | led_number >12)
		{
			display_msg_error();
		}
		/*check led state value */
		else if ((buff[i+1] !='1')&&(buff[i+1] !='0'))
		{
			display_msg_error();
		}
		else
		{
			
			if(buff[i+1] =='1')
			{
				/*turn on led*/
				*PORT[led_number ] &= _value_on[led_number];	
			}
			else if (buff[i+1] =='0')
			{
				/*turn off led*/
				*PORT[led_number] |= _value_off[led_number];	
			}
			/*asign flag value*/
			error_flag = NO_ERROR;
			P1_bit.no0 = 0;
		}
	}
}
/************************************************************************************
Function name :Send_led
Decription    :This function Convert and send led status to app
*************************************************************************************/
void Send_led(void)
{
	
	char buffer[13] ="$1872,L";
	uint8_t convert;
	/*take status led*/
 	status_led();
	convert = (uint8_t)((led_status_1.status_led & 0xF000) >> 12);
	buffer[7] = (convert < 0x0A) ? (convert+0x30):(convert+0x37);
	convert = (uint8_t)((led_status_1.status_led & 0x0F00) >> 8);
	buffer[8] = (convert < 0x0A) ? (convert+0x30):(convert+0x37);
	convert = (uint8_t)((led_status_1.status_led & 0x00F0) >> 4);
	buffer[9] = (convert < 0x0A) ? (convert+0x30):(convert+0x37);
	convert = (uint8_t)(led_status_1.status_led & 0x000F);
	buffer[10] = (convert < 0x0A) ? (convert+0x30):(convert+0x37);
	buffer[11] = '^';
	/*transmit led status*/
	Uart_Transmit(buffer,12);
}
