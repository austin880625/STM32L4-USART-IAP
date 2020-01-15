#ifndef	__IAPP_H
#define	__IAPP_H

#include "../stm/stm32l476xx.h"

#define IAPP_GET	1
#define IAPP_RUN	2
#define IAPP_RESET	3
#define IAPP_UPLOAD	4
#define IAPP_SET	5

#define VTOR_BASE_ADDR ((uint32_t *)0x08080000U)
#define USART_INT_ADDR ((uint32_t *)0x080000D4U)
#define OPTION_ADDR ((uint32_t *)0x080FF800)

struct __attribute__((__packed__)) iapp_header_t {
	uint32_t command;
};

void iapp_pre_reply(uint8_t *data, uint32_t size, uint8_t *reply);
void iapp_post_reply(uint8_t *data, uint32_t size);
int iapp_get_program_status();
void iapp_set_program_status(int status);
int iapp_get_command();

#endif
