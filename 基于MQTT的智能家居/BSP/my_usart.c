/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2025-09-23 20:41:48
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2025-09-27 14:56:12
 * @FilePath: \MDK-ARMd:\keil\project\FreeRTOS\MQTT\BSP\my_usart.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "my_usart.h"
#include "circule_buffer.h"
#include "platform_mutex.h"

static unsigned char rx_data;
static volatile uint8_t txcplt_flag = 0;
static volatile uint8_t rxcplt_flag = 0;

static ring_buffer uart_buffer_instance = {NULL};
extern ring_buffer test_buffer;
static platform_mutex_t xUART_Lock_mutex;

void UartLockInit(void)
{
	platform_mutex_init(&xUART_Lock_mutex);
    platform_mutex_lock(&xUART_Lock_mutex);
	ring_buffer_init(&uart_buffer_instance);
}

void UartRead(char *buf, uint32_t timeout)
{
	while(1)
	{
	if(0 == ring_buffer_read((unsigned char *)buf, &uart_buffer_instance))
		return;
	else
		platform_mutex_lock_timeout(&xUART_Lock_mutex, timeout);
	}
}
/* 发送中断 */
void USART3_Transmit(char *buf, int len)
{
	for(int i = 0; i < len; i++)
	{
		while((huart3.Instance->SR & USART_SR_TXE) == 0);
		huart3.Instance->DR = buf[i];
	}
}
///* 接收中断 */
void USART3_IRQHandler(void)
{
	/* 发生RX中断,把数据读出来，添加进环形buff */
	uint32_t isrflags = READ_REG(huart3.Instance->SR);
	uint32_t cr1its   = READ_REG(huart3.Instance->CR1);
	
	if (((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
    {
		rx_data = huart3.Instance->DR;
		/* 数组保存接收字符 */
		ring_buffer_write(rx_data, &uart_buffer_instance);
		/** 唤醒任务 */
		platform_mutex_unlockformISR(&xUART_Lock_mutex);	

		return;
    }
}

void USART3_IRQEnable(void)
{
	HAL_NVIC_SetPriority(USART3_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(USART3_IRQn);
	
	huart3.Instance->SR &= ~(USART_SR_RXNE);
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
}
