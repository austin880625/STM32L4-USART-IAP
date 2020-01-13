#ifndef	__USART_H
#define	__USART_H

#include "../stm/stm32l476xx.h"

struct __attribute__((__packed__)) packet_header_t {
	uint32_t checksum;
	uint32_t size;
	uint8_t control;
};

struct usart_cb {
	void (*pre_reply)(uint8_t*, uint32_t size, uint8_t *reply);
	void (*post_reply)(uint8_t*, uint32_t size);
};

void usart_send_byte(uint8_t x);
void usart_send(uint8_t *data, uint32_t len);
void usart_send_uint(uint32_t x);
void usart_send_packet(struct packet_header_t *header, uint8_t *payload);
uint32_t usart_recv(uint8_t *data, uint32_t len);
uint32_t usart_checksum(uint8_t *data, uint32_t len);
uint32_t usart_recv_uint();

void usart_register_cb(uint8_t port, struct usart_cb *cb);
void usart_dispatch_pre_reply(uint8_t port, uint8_t *payload, uint32_t size, uint8_t *reply);
void usart_dispatch_post_reply(uint8_t port, uint8_t *payload, uint32_t size);

#endif
