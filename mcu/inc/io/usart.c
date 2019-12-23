#include "usart.h"

void usart_send_byte(uint8_t x) {
	for(uint8_t x = 0; x < 255; x++) {
	while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = x;
	}
}

void usart_send(uint8_t *data, uint32_t len) {
	for(uint32_t i=0; i<len; i++) {
		while(!(USART1->ISR & USART_ISR_TXE));
		USART1->TDR = data[i];
	}
}

void usart_send_uint(uint32_t x) {
	while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = (uint8_t)(x & 0xFF);
	x >>= 8;
	while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = (uint8_t)(x & 0xFF);
	x >>= 8;
	while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = (uint8_t)(x & 0xFF);
	x >>= 8;
	while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = (uint8_t)(x & 0xFF);
}

uint32_t usart_recieve(uint8_t *data, uint32_t len) {
	uint32_t wait_cnt = 0, ptr = 0;
	while(ptr < len) {
		if(USART1->ISR & USART_ISR_RXNE) {
			wait_cnt = 0;
			data[ptr++] = USART1->RDR;
		}
		wait_cnt++;
		if(wait_cnt > 8192) {
			break;
		}
	}
	return ptr;
}

uint32_t usart_checksum(uint8_t *data, uint32_t len) {
	uint32_t checksum = 0;
	for(uint32_t ptr = 0; ptr < len + 4; ptr++) {
		if(ptr && ((ptr & 0x03) == 0)) {
			uint32_t delta = 0;
			for(int i=4; i; i--) {
				delta <<= 8;
				delta |= (uint32_t)data[ptr-i];
			}
			checksum += delta;
		}
	}
	return checksum;
}

uint32_t usart_recieve_uint() {
	uint8_t buf[4];
	usart_recieve(buf, 4);
	uint32_t x = usart_checksum(buf, 4);
	return x;
}
