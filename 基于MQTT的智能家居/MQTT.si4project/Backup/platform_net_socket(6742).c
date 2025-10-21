/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2020-01-10 23:45:59
 * @LastEditTime: 2025-10-02 20:15:17
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
 #include <stdio.h>
#include "platform_net_socket.h"
#include "at_commend.h"
#include "my_usart.h"

#define UserName     "Minerva"
#define UserPassword "ykf040505"
#define TIMEOUT      2000


int platform_net_socket_connect(const char *host, const char *port, int proto)
{
    int cmd_state = 0;
	char cmd[100];
	/* 配置WIFI模式 */
	cmd_state = ATSendCmd("AT+CWMODE=3", NULL, TIMEOUT);
	if(!cmd_state)
	{
		return cmd_state;
	}
	/* 连接热点 */
	cmd_state = ATSendCmd("AT+CWJAP=\"" UserName "\",\""UserPassword"\"", NULL, TIMEOUT);
	if(!cmd_state)
	{
		printf("connect AP error = %d\r\n", cmd_state);
		return cmd_state;
	}
	/* 连接服务器 */
	if(proto == PLATFORM_NET_PROTO_TCP)
	{
		snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%s", host, port);
	}
	else if(proto == PLATFORM_NET_PROTO_UDP)
	{
		snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"UDP\",\"%s\",%s", host, port);
	}
	cmd_state = ATSendCmd(cmd, NULL, TIMEOUT);
	if(!cmd_state)
	{
		printf("%s error = %d\r\n", cmd, cmd_state);
		return cmd_state;
	}

    return 0;
}
#if 0
int platform_net_socket_recv(int fd, void *buf, size_t len, int flags)
{
    return 0;
}
#endif
int platform_net_socket_recv_timeout(int fd, unsigned char *buf, int len, int timeout)
{
    
    return 0;
}
#if 0
int platform_net_socket_write(int fd, void *buf, size_t len)
{
    return 0;
}
#endif
int platform_net_socket_write_timeout(int fd, unsigned char *buf, int len, int timeout)
{
	int cmd_state = 0;
	char cmd_buf[20] = {NULL};

	snprintf(cmd_buf, sizeof(cmd_buf), "AT+CIPSEND=%d", len);
	cmd_state = ATSendCmd(cmd_buf, NULL, timeout);
	if (!cmd_state)
	{
		printf("AT+CIPSEND error = %d\r\n", cmd_state);
		return cmd_state;
	}
	
	cmd_state = ATSendData((char *)buf, timeout);	
	if (!cmd_state)
	{
		printf("data send error = %d\r\n", cmd_state);
		return cmd_state;
	}

   	return 0;
}
 
int platform_net_socket_close(int fd)
{
	ATSendCmd("AT+CIPCLOSE", NULL, TIMEOUT);
	return 0;
}
#if 0
int platform_net_socket_set_block(int fd)
{
    return 0;
}
 
int platform_net_socket_set_nonblock(int fd)
{
    return 0;
}
 
int platform_net_socket_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    return 0;
}
#endif
