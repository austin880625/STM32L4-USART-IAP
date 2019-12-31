#ifndef	__IAPP_H
#define	__IAPP_H

#include "../stm/stm32l476xx.h"

#define VTOR_BASE_ADDR ((uint32_t *)0x08080000U)

struct __attribute__((__packed__)) iapp_header_t {
	uint32_t command;
};

void iapp_pre_reply(uint8_t *data, uint32_t size, uint8_t *reply);
void iapp_post_reply(uint8_t *data, uint32_t size);
int iapp_program_status();

#endif
