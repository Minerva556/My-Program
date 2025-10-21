#ifndef __MY_USART_H
#define __MY_USART_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "usart.h"
#include "main.h"
#include "circule_buffer.h"

void UartRead(char *buf, uint32_t timeout);
void UartLockInit(void);
void USART3_Transmit(char *buf, int len);
void USART3_IRQEnable(void);
void EnableDebugIRQ(void);
	
#endif
