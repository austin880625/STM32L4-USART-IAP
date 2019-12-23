#include "usart.h"

#define base16m_encode(x) (x + 'A')
#define base16m_decode(bs) (((bs[0]-'A')<<4) | (bs[1]-'A'))

inline void usart_send_byte(uint8_t x) {
	uint8_t bl = (x >> 4), br = x & 0x0F;
	while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = base16m_encode(bl);
	while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = base16m_encode(br);
}

void usart_send(uint8_t *data, uint32_t len) {
	for(uint32_t i=0; i<len; i++) {
		usart_send_byte(data[i]);
	}
}

void usart_send_uint(uint32_t x) {
	while(!(USART1->ISR & USART_ISR_TXE));
	usart_send_byte((uint8_t)(x & 0xFF));
	x >>= 8;
	while(!(USART1->ISR & USART_ISR_TXE));
	usart_send_byte((uint8_t)(x & 0xFF));
	x >>= 8;
	while(!(USART1->ISR & USART_ISR_TXE));
	usart_send_byte((uint8_t)(x & 0xFF));
	x >>= 8;
	while(!(USART1->ISR & USART_ISR_TXE));
	usart_send_byte((uint8_t)(x & 0xFF));
}

uint32_t usart_receive(uint8_t *data, uint32_t len) {
	uint32_t wait_cnt = 0, ptr = 0;
	uint8_t bs[2];
	while(ptr < len) {
		uint32_t received = 0;
		while(!received) {
			if(USART1->ISR & USART_ISR_RXNE) {
				wait_cnt = 0;
				bs[0] = USART1->RDR;
				received = 1;
			}
			wait_cnt++;
			if(wait_cnt > 8192) {
				break;
			}
		}
		if(wait_cnt > 8192) break;
		wait_cnt = received = 0;
		while(!received) {
			if(USART1->ISR & USART_ISR_RXNE) {
				wait_cnt = 0;
				bs[1] = USART1->RDR;
				received = 1;
			}
			wait_cnt++;
			if(wait_cnt > 8192) {
				break;
			}
		}
		if(wait_cnt > 8192) break;
		data[ptr++] = base16m_decode(bs);
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

uint32_t usart_receive_uint() {
	uint8_t buf[4];
	usart_receive(buf, 4);
	uint32_t x = usart_checksum(buf, 4);
	return x;
}
