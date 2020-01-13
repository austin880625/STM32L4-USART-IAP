#include "iapp.h"
#include "usart.h"

struct iapp_state_t {
	int program_ready;
	int command;
};

static struct iapp_state_t iapp_state = {
	.program_ready = 0,
	.command = 0
};

void flash_write(uint32_t *data, uint32_t n) {
	while(FLASH->SR & FLASH_SR_BSY);
	FLASH->SR |= FLASH_SR_FASTERR | FLASH_SR_MISERR | FLASH_SR_PGSERR | FLASH_SR_SIZERR |
		FLASH_SR_PGAERR | FLASH_SR_WRPERR | FLASH_SR_PROGERR;
	FLASH->CR &= (~FLASH_CR_PER) & (~FLASH_CR_MER1) & (~FLASH_CR_MER2);
	// Erase the desired region
	uint32_t page = 256, n0 = n;
	FLASH->CR |= FLASH_CR_PER;
	do {
		FLASH->CR = (FLASH->CR & 0xFFFFF807) | (page << 3);
		FLASH->CR |= FLASH_CR_STRT;
		while(FLASH->SR & FLASH_SR_BSY);
	} while(n0 >>= 11);
	FLASH->SR |= FLASH_SR_FASTERR | FLASH_SR_MISERR | FLASH_SR_PGSERR | FLASH_SR_SIZERR |
		FLASH_SR_PGAERR | FLASH_SR_WRPERR | FLASH_SR_PROGERR;
	FLASH->CR &= (~FLASH_CR_PER) & (~FLASH_CR_MER1) & (~FLASH_CR_MER2);

	// Program sequence
	uint32_t *dst_addr = VTOR_BASE_ADDR;
	FLASH->CR |= FLASH_CR_PG;
	for(int i=0; i<n; i-=-2) {
		dst_addr[i] = data[i];
		dst_addr[i+1] = data[i+1];
		while(FLASH->SR & FLASH_SR_BSY);
		FLASH->SR |= FLASH_SR_EOP;
	}
	FLASH->CR &= (~FLASH_CR_PG);
}

void iapp_pre_reply(uint8_t *data, uint32_t size, uint8_t *reply) {
	struct iapp_header_t *iapp = (struct iapp_header_t *)data;
	data += (sizeof(struct iapp_header_t));
	struct packet_header_t *reply_header = (struct packet_header_t *)reply;
	uint8_t *payload = reply + sizeof(struct packet_header_t);
	uint32_t reply_size;

	iapp_state.command = iapp->command;
	if(iapp->command == IAPP_GET) {
		reply_size = size-sizeof(struct iapp_header_t);
		for(int i=0; i<reply_size; i++) {
			payload[i] = data[i];
		}
	} else if(iapp->command == IAPP_RESET) {
		SCB->AIRCR |= SCB_AIRCR_SYSRESETREQ_Msk;
	} else if(iapp->command == IAPP_RUN) {
		if(iapp_state.program_ready == 0) {
			reply_size = 3;
			payload[0] = 'N';
			payload[1] = 'O';
			payload[2] = '\0';
		} else {
			reply_size = 3;
			payload[0] = 'O';
			payload[1] = 'K';
			payload[2] = '\0';
		}
	} else if(iapp->command == IAPP_UPLOAD) {
		flash_write((uint32_t *)data, size);
		iapp_state.program_ready = 1;
		reply_size = 3;
		payload[0] = 'O';
		payload[1] = 'K';
		payload[2] = '\0';
	}
	while((reply_size&0x11))payload[reply_size++] = '\0';;
	reply_header->checksum = usart_checksum(payload, reply_size) + reply_size;
	reply_header->size = reply_size;
	// 0x02 ==> the last packet
	reply_header->control = 0x02;
}

void iapp_post_reply(uint8_t *data, uint32_t size) {
}

int iapp_program_status() {
	return iapp_state.program_ready;
}

int iapp_get_command() {
	return iapp_state.command;
}
