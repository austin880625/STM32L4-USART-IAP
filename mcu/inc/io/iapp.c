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

void flash_write(uint32_t *data, uint32_t n, uint32_t *dst_addr, uint32_t page) {
	while(FLASH->SR & FLASH_SR_BSY);
	FLASH->SR |= FLASH_SR_FASTERR | FLASH_SR_MISERR | FLASH_SR_PGSERR | FLASH_SR_SIZERR |
		FLASH_SR_PGAERR | FLASH_SR_WRPERR | FLASH_SR_PROGERR;
	FLASH->CR &= (~FLASH_CR_PER) & (~FLASH_CR_MER1) & (~FLASH_CR_MER2);
	// Erase the desired region
	int n0 = n;
	FLASH->CR |= FLASH_CR_PER;
	do {
		FLASH->CR = (FLASH->CR & 0xFFFFF807) | (page << 3);
		FLASH->CR |= FLASH_CR_STRT;
		while(FLASH->SR & FLASH_SR_BSY);
		n0 -= 2048;
		page++;
	} while(n0 > 0);
	FLASH->SR |= FLASH_SR_FASTERR | FLASH_SR_MISERR | FLASH_SR_PGSERR | FLASH_SR_SIZERR |
		FLASH_SR_PGAERR | FLASH_SR_WRPERR | FLASH_SR_PROGERR;
	FLASH->CR &= (~FLASH_CR_PER) & (~FLASH_CR_MER1) & (~FLASH_CR_MER2);

	// Program sequence
	FLASH->CR |= FLASH_CR_PG;
	n >>= 2;
	for(int i=0; i<=n; i-=-2) {
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
		uint32_t option = *(OPTION_ADDR);
		reply_size = 4;
		*((uint32_t *)payload) = option;
	} else if(iapp->command == IAPP_SET) {
		uint32_t new_option = *((uint32_t *)data);
		uint32_t option = *(OPTION_ADDR);
		new_option = (new_option & (~0x02)) | (option & 0x02);
		flash_write(&new_option, 4, OPTION_ADDR, 511);
		reply_size = 4;
		*((uint32_t *)payload) = new_option;
	} else if(iapp->command == IAPP_RESET) {
		reply_size = 3;
		payload[0] = 'O';
		payload[1] = 'K';
		payload[2] = '\0';
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
		*(uint32_t *)(&data[212]) = *(USART_INT_ADDR);
		flash_write((uint32_t *)data, size, VTOR_BASE_ADDR, 256);
		// 0x02 : if the program exist on the flash
		// 0x01 : should the program be executed directly
		uint32_t option = 0x0;
		flash_write(&option, 4, OPTION_ADDR, 511);
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

int iapp_get_program_status() {
	return iapp_state.program_ready;
}
void iapp_set_program_status(int status) {
	iapp_state.program_ready = status;
}

int iapp_get_command() {
	return iapp_state.command;
}
