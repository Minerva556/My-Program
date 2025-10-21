/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2025-09-27 09:33:51
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2025-10-02 20:29:05
 * @FilePath: \MDK-ARMd:\keil\project\FreeRTOS\MQTT\BSP\at_commend.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "at_commend.h"
#include "abstraction_hardware.h"
#include "platform_mutex.h"

#define AT_RESP_LEN     256
#define AT_COMMEND_SIZE 64

static ring_buffer g_packet_buf;


static int g_at_state = 0;
static int g_at_packet_len = 0;
static char recv_buf[AT_RESP_LEN] = {NULL};
static char packet_buf[AT_RESP_LEN] = {NULL};

static platform_mutex_t xAT_ret_mutex;
static platform_mutex_t xAT_packet_mutex;

static void SetATStates(int state)
{
    g_at_state = state;
}
static int GetATStates(void)
{
    return g_at_state;
}
 void ATInit(void)
 {
    platform_mutex_init(&xAT_ret_mutex);
    platform_mutex_lock(&xAT_ret_mutex);/** mutex = 0 */

    platform_mutex_init(&xAT_packet_mutex);
    platform_mutex_lock(&xAT_packet_mutex);/** mutex = 0 */
 }
static int ATGetSpecailString(char *buf)
{
    char *spec_IPD = "+IPD,";

    if(strstr(buf, spec_IPD))
    {
        return AT_OK;
    }
	return AT_ERROR;
}
static void ProcessSpecialData(char *buf)
{
    char spec_data = ':';
    int spec_data_len = 0;
    int spec_data_index = 0;
	
    while(1)
	{
		while(1)
		{
			spec_data_index = 0;
			AbstractReceive(&buf[spec_data_index], portMAX_DELAY);
			if (buf[spec_data_index] == spec_data)
			{
				break;
			}
			else
			{
				spec_data_len = spec_data_len * 10 + (buf[spec_data_index] - '0');
			}
			spec_data_index++;
		}
			
		/* 读取真正的数据 */
		spec_data_index = 0;
		while (spec_data_index < spec_data_len)
		{
			AbstractReceive(&buf[spec_data_index], portMAX_DELAY);
			if (spec_data_index < AT_RESP_LEN)
			{
				ring_buffer_write(buf[spec_data_index], &g_packet_buf);
				platform_mutex_unlock(&xAT_packet_mutex);
			}
			spec_data_index++;
		}
	}   
}
static int ATGetCIPSEND(char *buf)
{
    char *spec_CIPSEND = ">";

    if(strstr(buf, spec_CIPSEND))
    {
        return AT_OK;
    }
	return AT_ERROR;
}

int ATReadPacket(char *buf, int len, char *respon_len, uint32_t timeout)
{
    int ret = 0;
    ret = platform_mutex_lock_timeout(&xAT_packet_mutex, timeout);
    if(ret)
    {
        *respon_len = len > g_at_packet_len ? g_at_packet_len : len;
        /** 将获得的数据拷贝到用户提供的缓冲区 */
        memcpy(buf, packet_buf, *respon_len);

        return AT_OK;
    }
    else
    {
        return AT_TIMEOUT;
    }
}
int ATSendData(char *buf, uint32_t timeout)
{
    int ret = 0;
    int at_state = 0;
 
    AbstractSend(buf, sizeof(buf));

    ret = platform_mutex_lock_timeout(&xAT_ret_mutex, timeout);
    if(ret)
    {
        at_state = GetATStates();
        //接收返回值
        return at_state;
    }
    else
    {
        /** 超时返回 */
        return AT_TIMEOUT;
    } 
}

int ATReadData(unsigned char *buf, int timeout)
{
	int ret = 0;

	do
	{
		if(0 == ring_buffer_read((unsigned char *)buf, &g_packet_buf))
		{
			return AT_OK;
		}
		else
		{
			/* 环形缓冲区为空，则阻塞 */
			ret = platform_mutex_lock_timeout(&xAT_packet_mutex, timeout);
			if(ret == 0)
			{
				return AT_TIMEOUT;
			}
		}
	
	}while(ret == 1);

	return 0;
}

/**
 * @brief 向ESP8266发送AT指令
 * @attention if you want to send commend, you mustn't send '\r\n'
 *            timeout: ms  
 * @return NULL
 */
int ATSendCmd(char *buf, char *respon, uint32_t timeout)
{
    int ret = 0;
    int at_state = 0;
	int cmd_len = 0;
	int copy_len = 0;
	char cmd_buf[AT_COMMEND_SIZE] = {NULL};
	
    if(!strstr(buf, "AT+CIPSEND="))
    {
        cmd_len = snprintf(cmd_buf, sizeof(cmd_buf), "%s\r\n", buf);
        /* 发送AT指令 */
        AbstractSend(cmd_buf, cmd_len); 
    }
    else
    {
        AbstractSend(buf, strlen(buf));
    }

    ret = platform_mutex_lock_timeout(&xAT_ret_mutex, timeout);
    if(ret)
    {
        at_state = GetATStates();
        //接收返回值
        if(!at_state && respon)
        {
			copy_len = strlen(respon) > AT_RESP_LEN ? AT_RESP_LEN : strlen(respon);
			/** 将获得的数据拷贝到用户提供的缓冲区 */
			memcpy(respon, recv_buf, copy_len);
        }
        return at_state;
    }
    else
    {
        /** 超时返回 */
        return AT_TIMEOUT;
    } 
}

void ATReceParse(void *params)
{
    int buf_len = 0;
    
    char *receive_success = "OK";
    char *receive_fail = "ERROR";
    char parse_buf[AT_RESP_LEN] = {NULL};

    while (1)
    {
        /** 读环形缓冲区 */
        AbstractReceive(&parse_buf[buf_len], portMAX_DELAY);       
      
        if(buf_len && (parse_buf[buf_len - 1] == '\r') && (parse_buf[buf_len] == '\n'))
        {    
            parse_buf[buf_len + 1] = '\0';
            /** 接收到成功 */
            if(strstr(parse_buf, receive_success))
            {
                /* 将接收到的数据复制到recv_buf */
                strncpy(recv_buf, parse_buf, buf_len);
                SetATStates(AT_OK);
                platform_mutex_unlock(&xAT_ret_mutex);
				buf_len = 0;
            }
            /** 接收到失败 */
            else if(strstr(parse_buf, receive_fail))
            {
				strncpy(recv_buf, parse_buf, buf_len);
                SetATStates(AT_ERROR);
				platform_mutex_unlock(&xAT_ret_mutex);
				buf_len = 0;
            }
            /** 接收到AT+CIPSEND */
            else if(ATGetCIPSEND(parse_buf) == AT_OK)
            {
                SetATStates(AT_OK);
                platform_mutex_unlock(&xAT_ret_mutex);
                buf_len = 0;
            }
            /** 接收到特殊字符 */
            else if(ATGetSpecailString(parse_buf) == AT_OK)
            {
                ProcessSpecialData(parse_buf);  
				buf_len = 0;				
            }else
			{
				buf_len++;
			}
        }
        else
		{
			buf_len++;
		}
        if(buf_len >= AT_RESP_LEN)
            buf_len = 0;
    }  
}
/* 测试任务 */
void Task_test(void *params)
{
	int ret = 0;
	char *commend = "AT+CIPSEND=4";
	char buf[AT_RESP_LEN];
	
	while(1)
	{
		ret = ATSendCmd(commend, buf, 1000);
				
		printf("commend state = %d\r\n", ret);
		printf("commend return states:\r\n");
		printf(buf);
		printf("\n");	
		printf("--------------------------------------------");
		printf("\r\n");
		
		vTaskDelay(2000);
	}
}
