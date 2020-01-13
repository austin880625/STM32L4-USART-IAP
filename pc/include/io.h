#ifndef __IO_H
#define __IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>

struct __attribute__((__packed__)) packet_header_t {
	uint32_t checksum;
	uint32_t size;
	uint8_t control;
};

void recv_byte(int, uint8_t*);
void recv_nbyte(int, uint8_t*, int);
void recv_packet_header(int, struct packet_header_t*);
void recv_packet(int, struct packet_header_t*, uint8_t*);

void send_byte(int, uint8_t*);
void send_nbyte(int, uint8_t*, int);
void send_packet_header(int, struct packet_header_t*);
void send_packet(int, struct packet_header_t*, uint8_t*);

uint32_t cal_checksum(uint8_t*, size_t);
void dump(const char*, uint8_t *, size_t, size_t);
uint8_t* concat(uint8_t*, size_t, uint8_t*, size_t);

#endif
