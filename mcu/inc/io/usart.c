#include "usart.h"

#define base16m_encode(x) (x + 'A')
#define base16m_decode(bs) (((bs[0]-'A')<<4) | (bs[1]-'A'))
#define USART_TIMEOUT 1310720

static struct usart_cb registered_cb[2];

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
/*
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
*/

void usart_send_packet(struct packet_header_t *header, uint8_t *payload) {
	usart_send((uint8_t*)header, sizeof(struct packet_header_t));
	usart_send(payload, header->size);
}

uint32_t usart_recv(uint8_t *data, uint32_t len) {
	uint32_t wait_cnt = 0, ptr = 0;
	uint8_t bs[2];
	while(ptr < len) {
		uint32_t recvd = 0;
		while(!recvd) {
			if(USART1->ISR & USART_ISR_RXNE) {
				wait_cnt = 0;
				bs[0] = USART1->RDR;
				recvd = 1;
			}
			wait_cnt++;
			if(wait_cnt > USART_TIMEOUT) {
				break;
			}
		}
		if(wait_cnt > USART_TIMEOUT) break;
		wait_cnt = recvd = 0;
		while(!recvd) {
			if(USART1->ISR & USART_ISR_RXNE) {
				wait_cnt = 0;
				bs[1] = USART1->RDR;
				recvd = 1;
			}
			wait_cnt++;
			if(wait_cnt > USART_TIMEOUT) {
				break;
			}
		}
		if(wait_cnt > USART_TIMEOUT) break;
		data[ptr++] = base16m_decode(bs);
	}
	return ptr;
}

uint32_t usart_checksum(uint8_t *data, uint32_t len) {
	uint32_t checksum = 0;
	for(uint32_t ptr = 0; ptr < len; ptr+=4) {
		uint32_t delta = *((uint32_t*)(data+ptr));
		checksum += delta;
	}
	return checksum;
}

uint32_t usart_recv_uint() {
	uint8_t buf[4];
	usart_recv(buf, 4);
	uint32_t x = usart_checksum(buf, 4);
	return x;
}

void usart_register_cb(uint8_t port, struct usart_cb *cb) {
	registered_cb[port].pre_reply = cb->pre_reply;
	registered_cb[port].post_reply = cb->post_reply;
}

void usart_dispatch_pre_reply(uint8_t port, uint8_t *payload, uint32_t size, uint8_t *reply) {
	if(!(registered_cb[port].pre_reply))return ;
	registered_cb[port].pre_reply(payload, size, reply);
}

void usart_dispatch_post_reply(uint8_t port, uint8_t *payload, uint32_t size) {
	if(!(registered_cb[port].post_reply))return ;
	registered_cb[port].post_reply(payload, size);
}
