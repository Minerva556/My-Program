#ifndef __AT_COMMEND_H__
#define __AT_COMMEND_H__

#include <stdint.h>

#define AT_OK               0
#define AT_ERROR           -1
#define AT_TIMEOUT         -2
#define DATA_OUT		   -3

void ATInit(void);
int ATReadPacket(char *buf, int len, char *respon_len, uint32_t timeout);
int ATSendCmd(char *buf, char *respon, uint32_t timeout);
int ATSendData(char *buf, uint32_t timeout);
void ATReceParse(void *params);
void Task_test(void *params);

#endif
