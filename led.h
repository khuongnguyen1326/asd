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
* File Name    : led.h
* Version      : 1.00
* Description  : 
******************************************************************************
* History : DD.MM.YYYY Version Description
*         : 01.07.2015 1.00    First Release
******************************************************************************/

#ifndef __LED_PORT
#define __LED_PORT

#define PORT6 (volatile unsigned char *)0xFFF06
#define PORT4 (volatile unsigned char *)0xFFF04
#define PORT10 (volatile unsigned char *)0xFFF0A
#define PORT15 (volatile unsigned char *)0xFFF0F

#endif
/*define value of led*/
#define LED3_ON 	0xFB
#define LED5_ON 	0xF5
#define LED7_ON 	0xEF
#define LED9_ON 	0xDF
#define LED11_ON 	0xBF
#define LED13_ON	0x7F

#define LED15_ON 	0xFD
#define LED4_ON 	0xFB
#define LED6_ON 	0xF7
#define LED8_ON 	0xEF
#define LED10_ON 	0xDF
#define LED12_ON	0xFB
#define LED14_ON 	0xFD

#define LED3_OFF 	0x04
#define LED5_OFF	0x08
#define LED7_OFF 	0x10
#define LED9_OFF	0x20
#define LED11_OFF 	0x40
#define LED13_OFF	0x80

#define LED15_OFF 	0x02
#define LED4_OFF 	0x04
#define LED6_OFF	0x08
#define LED8_OFF 	0x10
#define LED10_OFF 	0x20
#define LED12_OFF	0x04
#define LED14_OFF	0x02


extern unsigned char *PORT[13];	     
extern unsigned char _value_on[13];	
extern unsigned char _value_off[13];
extern char tx_buff_led[];
typedef union struct_status{
	unsigned short status_led;
	struct STRUCT_BIT{
		unsigned short bit0:1;
		unsigned short bit1:1;
		unsigned short bit2:1;
		unsigned short bit3:1;
		
		unsigned short bit4:1;
		unsigned short bit5:1;
		unsigned short bit6:1;
		unsigned short bit7:1;
		
		unsigned short bit8:1;
		unsigned short bit9:1;
		unsigned short bit10:1;
		unsigned short bit11:1;
		
		unsigned short bit12:1;
		unsigned short bit13:1;
		unsigned short bit14:1;
		unsigned short bit15:1;
	}BIT;
}led_status;
extern led_status led_status_1; 


void ledinit(void);
void status_led(void);
void check_led_status(char *buff);
void Send_led(void);
