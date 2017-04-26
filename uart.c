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
* File Name    : uart.c
* Version      : Initial version 1.0.0
* Device(s)    : R5F104PJ
* Tool-Chain   : CA78K0R
* Description  : This file implements device driver for UART module.
* Creation Date: 09-Sep-15
***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#pragma interrupt INTSR1 UartRx_isr

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_macro.h"
#include "uart.h"
#include "led.h"
#include "lcd.h"
#include "stdlib.h"
#include <string.h>
/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
char receive_data[9][UART_RX_BUFFER_LEN];
char temp_receive[UART_RX_BUFFER_LEN];
unsigned int rx_Len;
unsigned char status = 0;
int numb =0;
char cmd;
unsigned char LCD_Line;
unsigned int count = 0;
extern int error_flag;
extern int index;
extern int lcd_status;
int check_content;
int check_special;
/***********************************************************************************************************************
* Function Name: Uart_Init
* Description  : This function initializes the UART1.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void Uart_Init(void)
{
	/* Uart init UART0 */
	SAU0EN = 1U;   /*serial array unit 0 Enable*/
	NOP();
	NOP();
	NOP();
	NOP();
	SPS0 = 0x0004|0x0040; /*select clock for SAU*/

	ST0 = 0x04;      /* stop all channel */
	STMK1 = 1U;      /* disable INTST0 interrupt */
	STIF1 = 0U;      /* clear INTST0 interrupt flag */
	SRMK1 = 1U;      /* disable INTSR0 interrupt */
	SRIF1 = 0U;      /* clear INTSR0 interrupt flag */
	SREMK1 = 1U;     /* disable INTSRE0 interrupt */
	SREIF1 = 0U;     /* clear INTSRE0 interrupt flag */
	/* Set INTST0 low priority */
	STPR11 = 1U;
	STPR01 = 1U;
	/* Set INTSR0 low priority */
	SRPR11 = 1U;
	SRPR01 = 1U;

	SMR02 = 0x0022;   /* UART mode, empty buffer interrupt*/
	SCR02 = 0x8397;   /*Transmit mode, mask error interrupt, no parity, MSB first, stop bit 1 bit, data length 8 bit*/
	SDR02 = 0x3400U;  /*set clock transfer*/
	NFEN0 = 0x04;     /*noise enable for RXD0*/

	SIR03 = 0x07;     /*clear flag trigger */
	SMR03 = 0x0122;
	SCR03 = 0x4397;  /*Transmit mode, mask error interrupt, no parity, MSB first, stop bit 1 bit, data length 8 bit*/
	SDR03 = 0x3400;   /*set clock transfer*/

	SO0 |= 0x04;     /* output level normal */
	SOL0 |= 0x00;    /* output level normal */
	SOE0 |= 0x04;    /* enable UART1 output */

	/* Port inint for UART communicate */
	PMC0 &= 0xF7U;
	PM0 |= 0x08U;
	/* Set TxD1 pin */
	PMC0 &= 0xFBU;
	P0 |= 0x04U;
	PM0 &= 0xFBU;
}

/***********************************************************************************************************************
* Function Name: Uart_Start
* Description  : This function start UART1 operation.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void Uart_Start(void)
{
	SO0 |= 0x04;     /* output level normal */
	SOE0 |= 0x04;    /* enable UART0 output */
	SS0 |= 0x04|0x08;     /* enable UART0 receive and transmit */

	STIF1 = 0U;    /* clear INTST0 interrupt flag */
	SRIF1 = 0U;    /* clear INTSR0 interrupt flag */
	STMK1 = 1U;    /* disable INTST0 interrupt */
	SRMK1 = 0U;    /* enable INTSR0 interrupt */
}

/***********************************************************************************************************************
* Function Name: Uart_Stop
* Description  : This function stop UART1 operation.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void Uart_Stop(void)
{
	STMK1 = 1U;    /* disable INTST0 interrupt */
	SRMK1 = 1U;    /* disable INTSR0 interrupt */

	ST0 |= 0x04;     /* enable UART0 receive and transmit */
	SOE0 &= ~0x04;    /* enable UART0 output */

	STIF1 = 0U;    /* clear INTST0 interrupt flag */
	SRIF1 = 0U;    /* clear INTSR0 interrupt flag */
}

/***********************************************************************************************************************
* Function Name: Uart_Transmit
* Description  : This function transmit number of data bytes, using UART.
* Arguments    : Transmission data pointer, data length.
* Return Value : None
***********************************************************************************************************************/
void Uart_Transmit(const char* tx_ptr, int Len)
{
	unsigned int LuiCount = 0;
	unsigned int LuiTimeOut = 0;

	for(LuiCount = 0; LuiCount < Len; LuiCount++)
	{
	TXD1 = *(tx_ptr + LuiCount);
	LuiTimeOut = UART_TIMEOUT;
	while((STIF1 == 0)||(LuiTimeOut--));
	STIF1 = 0;
	}
}

/***********************************************************************************************************************
* Function Name: Uart_ClearRxBuff
* Description  : This function clear Rx buffer of UART
* Arguments    : Buffer pointer, buffer length.
* Return Value : None
***********************************************************************************************************************/
void Uart_ClearBuff(char* buff_ptr, unsigned int Len)
{
	unsigned int LuiCount = 0;
	for(LuiCount = 0; LuiCount < Len; LuiCount ++)
	{
	*(buff_ptr + LuiCount) = '\0';
	}
 
}

/***********************************************************************************************************************
* Function Name: UartRx_isr
* Description  : This interrupt service routine of UART1
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt void UartRx_isr(void)
{
	if (check_special==1)
	{
	  if(RXD1!= '$')
	  {
		error_flag = MSG_ERROR;
	  }
	  else
	  {
		check_special=0;  
	  }
	}

	if('^' == RXD1)
	{
	  if(count ==0)
	  {
		error_flag = MSG_ERROR;
	  }
	    
	    count = 0;    			
	    numb++;
	    check_special=1;
	    status = UART_RECEIVE_DONE;
	  
	  
	}

	else
	{
		receive_data[numb][count] = RXD1;
		count++;

		if(count > (UART_RX_BUFFER_LEN-2))
		{
			count = 0;
		}
		else
		{
			/* Do no thing */
		}

		if (numb >7 )
		{
			numb =0;
		}
		index =0;
	}

  
  
}

/***********************************************************************************************************************
* Function Name: check_receive_buff
* Description  : This function check data receive control led or display text
* Arguments    : char* buff
* Return Value : None
***********************************************************************************************************************/

void check_receive_buff(char* data)
{
	if ((error_flag != UART_ERROR)&&(error_flag != MSG_ERROR))
	{
		if (data[1] =='L')
		{
			check_led_status(data);
		}
		else if (data[1] =='T')
		{
			Display_text(data);
		}
		/*check format message*/
		else
		{
			display_msg_error();
		}
		
	}
}

/***********************************************************************************************************************
* Function Name: Display_text
* Description  : This function is used to check valid of text
* Arguments    : char* buff
* Return Value : None
***********************************************************************************************************************/

void Display_text(char* buffer)
{
	
	char temp_line[3]= {'\0','\0','\0'};
	char temp_text[12] ;
	int lcd_line;
	int i =0;
	status = 0;
	/*take line of text*/
	for (i =3 ;buffer[i] !=',';i++)
	{
		temp_line[i-3] = buffer[i];
		
	}
	
	lcd_line =  atoi(temp_line);
	lcd_line =lcd_line -1;
	/*check valid of line number*/
	if (lcd_line <0|lcd_line>8)
	{
		display_msg_error();
	}
	else
	{	
		/*check invalid length Message*/
		if (buffer[20] != '\0')
		{
			display_msg_error();
		}
		else
		{	for (i=5 ; i<18;i++)
			{
				/*take content*/
				temp_text[i-5] = buffer[i];
				/*check content of message*/
				
				if (buffer[i] !='\0')
				{
					if(((buffer[i]>='0')&&(buffer[i]<='9'))
					||((buffer[i]>='A')&&(buffer[i]<='Z'))
					||((buffer[i]>='a')&&(buffer[i]<='z'))
					||(buffer[i]==' ')||(buffer[i]=='_'))
					{
						
					}
					else
					{
						display_msg_error();
					}
				}
				else 
				{
					
				}
				
			}
			if(error_flag == NO_ERROR)
			{
				/*display data*/
				LCD_Line = (unsigned char)(lcd_line*8);
				DisplayLCD(LCD_Line, (uint8_t *)temp_text);
			}
		}
	}
		
		
}

/***********************************************************************************************************************
* Function Name: check_error
* Description  : This function is used to check valid of data receive
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void check_error_uart()
{
	if(error_flag != MSG_ERROR)
	{
		/*********Check UART error*********************/
	      if(SSR03&0x07)
	      {
	         /* Detected parity error */
	         if(SSR03 & ERR_PARITY_MASK)
	         {
	            display_uart_error();							
	         }
	         /* Detected framing error */
	         if(SSR03 & ERR_FRAMING_MASK)
	         {
		    display_uart_error();	
	         }
		
	      }  
	}
}

/************************
Function name : display data
Decription    : show to lcd 
Parameter     : 
***********************/

void display_data()
{
		int i;
		while(receive_data[index][0] != '\0' )
			{
				/*copy data to temp variable*/
				strcpy(temp_receive, receive_data[index]);
				/*check content of temp variable*/
				check_receive_buff(temp_receive);
				if ((error_flag == UART_ERROR)|(error_flag == MSG_ERROR))
				{
					break;
				}
				/*clear buffer and temp content*/
				Uart_ClearBuff(&temp_receive[0], UART_RX_BUFFER_LEN - 1);
				Uart_ClearBuff(receive_data[index], UART_RX_BUFFER_LEN - 1);
				index++;
			}
		/*all data has executed*/
		lcd_status =1;
		for(i=0; i <9 ; i ++)
		{
			Uart_ClearBuff(receive_data[i], UART_RX_BUFFER_LEN - 1);
		}
		numb =0;
		if (index>8)
		{
			index =0;
		}
}

/************************
Function name : display msg_error
Decription    : show to lcd 
Parameter     : 
***********************/
void display_msg_error()
{
	error_flag = MSG_ERROR;
	/*clear lcd*/
	ClearLCD();
	/* Display error message */
	DisplayLCD(LCD_LINE8, (uint8_t *)"Msg Err");
	/*turn on led 2*/
	P1_bit.no0 = 1;
	SIR03 = 0x07;
}
/************************
Function name : display msg_error
Decription    : show to lcd 
Parameter     : 
***********************/
void display_uart_error()
{
	    check_special=0;
	    /*clear LCD*/
            ClearLCD();
	     /*stop uart*/
	    Uart_Stop();
	    error_flag= UART_ERROR;
	    /*turn on led 2*/
	    P1_bit.no0 = 1;
	    SIR03 = 0x07;
	    DisplayLCD(LCD_LINE8, (uint8_t *)"UART Err");
}
/******************************************************************************
End of file
******************************************************************************/
