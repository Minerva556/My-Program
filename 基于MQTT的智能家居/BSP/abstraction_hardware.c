#include "abstraction_hardware.h"

void AbstractSend(char *buf, int len)
{
    USART3_Transmit(buf, len);
}

void AbstractReceive(char *buf, uint32_t timeout)
{
    UartRead(buf, timeout);
}
