#ifndef	__USART_H
#define	__USART_H

#include "../stm/stm32l476xx.h"

void usart_send_byte(uint8_t x);
void usart_send(uint8_t *data, uint32_t len);
void usart_send_uint(uint32_t x);
uint32_t usart_recieve(uint8_t *data, uint32_t len);
uint32_t usart_checksum(uint8_t *data, uint32_t len);
uint32_t usart_recieve_uint();

#endif
