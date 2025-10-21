/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2025-09-27 09:44:41
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2025-09-27 13:37:11
 * @FilePath: \MDK-ARMd:\keil\project\FreeRTOS\MQTT\BSP\abstraction_hardware.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __ABSTRACTION_HARDWARE_H
#define __ABSTRACTION_HARDWARE_H

#include "my_usart.h"

void AbstractSend(char *buf, int len);
void AbstractReceive(char *buf, uint32_t timeout);

#endif
