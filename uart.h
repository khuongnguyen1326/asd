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
* File Name    : uart.h
* Version      : 1.0.0
* Device(s)    : R5F104PJ
* Tool-Chain   : CA78K0R
* Description  : This file implements device driver for UART module.
* Creation Date: 09-Sep-15
***********************************************************************************************************************/

#ifndef UART_H
#define UART_H

/*******************************************************************************
* Macro Definitions
*******************************************************************************/
#define UART_TIMEOUT        1000U
#define UART_RECEIVE_DONE   1U
#define UART_CLEAR          2U
#define UART_RX_BUFFER_LEN  25U
#define UART_ERROR	   100U
#define MSG_ERROR	   200U

#define NO_ERROR	  10U
#define ERR_PARITY_MASK   0x0002       // Parity error mask
#define ERR_FRAMING_MASK  0x0004       // Framing error mask
/******************************************************************************
* Global Variable
******************************************************************************/
extern unsigned char status;
extern char cmd;
extern char receive_data[9][UART_RX_BUFFER_LEN];
extern int numb;
extern int error_flag;

/******************************************************************************
* Global Function Prototypes
******************************************************************************/

void Uart_Init(void);
void Uart_Start(void);
void Uart_Stop(void);
void Uart_Transmit(const char* tx_ptr, int Len);
void Uart_ClearBuff(char* buff_ptr, unsigned int Len);
void check_receive_buff(char* buff);
void Display_text(char* buff);
void check_error_uart(void);
void display_data(void);
void display_msg_error(void);
void display_uart_error(void);
#endif

/******************************************************************************
End of file
******************************************************************************/
