#ifndef __IAPP_H
#define __IAPP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define GET		1
#define RUN		2
#define RESET	3
#define UPLOAD	4
#define SET     5

struct __attribute__((__packed__)) iapp_header_t {
	uint32_t command;
};

size_t iapp(int, struct iapp_header_t*, uint8_t*, size_t, uint8_t*);
void dump_iapp(struct iapp_header_t*, uint8_t*, size_t);

#endif
