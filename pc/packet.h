#ifndef __PACKET_H
#define __PACKET_H

struct packet_header_t {
	char port;
	char last;
	uint16_t size;
	uint32_t chk;
	uint16_t seq;
	uint16_t ack;
} __attribute__((packed));

#endif
