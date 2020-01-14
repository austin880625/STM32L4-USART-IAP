#include "inc/stm/stm32l476xx.h"
#include "inc/io/usart.h"
#include "inc/io/iapp.h"

// use this pragma at handlers
//#pragma thumb

uint32_t assembled_size = 0;
uint8_t usart_buf[8192];
uint8_t usart_reply[128];

void flash_init() {
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;
}

#pragma thumb
void USART1_Handler() {
	NVIC->ICPR[1] = 0x20;
	NVIC->ICER[1] = 0x20;
	struct packet_header_t header;
	usart_recv((char *)&header, sizeof(struct packet_header_t));
	//const uint32_t checksum = usart_recv_uint();
	uint32_t checksum_self = 0;
	//uint32_t size = usart_recv_uint();
	checksum_self += header.size;
	uint32_t size_self = usart_recv(usart_buf + assembled_size, header.size);
	if(header.size == size_self) {
		checksum_self += usart_checksum(usart_buf + assembled_size, header.size);
	}
	//usart_send_uint(header.checksum);
	//usart_send_uint(checksum_self);
	if(header.checksum == checksum_self && header.size == size_self) {
		assembled_size += header.size;
		if(header.control & 0x02) {
			usart_dispatch_pre_reply(0, usart_buf, assembled_size, usart_reply);
			assembled_size = 0;
			usart_send_packet((struct packet_header_t *)usart_reply, usart_reply + sizeof(struct packet_header_t));
			int d = 2048;
			while(d--);
			if(iapp_get_command() == IAPP_RESET) {
				SCB->AIRCR = ((0x5FA << 16) | SCB_AIRCR_SYSRESETREQ_Msk);
			}
		} else {
			struct packet_header_t reply = { .checksum = 0, .size = 0, .control = 0 };
			usart_send_packet(&reply, 0);
		}
		NVIC->ICPR[1] = 0x20;
		NVIC->ISER[1] |= 0x20;
	} else {
		struct packet_header_t reply = { .checksum = 0, .size = 0, .control = 1 };
		usart_send_packet(&reply, 0);
		NVIC->ICPR[1] = 0x20;
		NVIC->ISER[1] |= 0x20;
	}
}

void GPIO_init() {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOCEN;
	GPIOA->MODER = (GPIOA->MODER & 0xFFC3FFFF) | 0x280000;
	GPIOA->AFR[1] = (GPIOA->AFR[1] & 0xFFFFF00F) | 0x770;
	GPIOA->MODER = (GPIOA->MODER & 0xFFFFF3FF) | 0x400;

	GPIOC->MODER = (GPIOC->MODER & 0xF3FFFFFF);
	GPIOC->OSPEEDR = 0x800;
	GPIOC->PUPDR = 0xAA;
}

void USART_init() {
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	USART1->BRR = 0x1A0;
	USART1->CR1 |= USART_CR1_RXNEIE;
	USART1->CR1 |= USART_CR1_TE;
	USART1->CR1 |= USART_CR1_RE;
	USART1->CR1 |= USART_CR1_UE;
}

void NVIC_init() {
	// NVIC_EnableIRQ(USART1_IRQn);
	NVIC->ISER[1] = 0x20;
	NVIC_SetPriority(37, 2);
}

int main() {
	GPIO_init();
	flash_init();
	USART_init();
	NVIC_init();
	struct usart_cb iapp_cb = {
		.pre_reply = iapp_pre_reply,
		.post_reply = iapp_post_reply
	};
	usart_register_cb(0, &iapp_cb);
	uint32_t *new_vtor = VTOR_BASE_ADDR + 1;
	void (*fn)() = *((void (**)())(new_vtor));
	uint32_t option = *(OPTION_ADDR);
	int cut_through = ((option & 0x01) == 1);
	if((option & 0x02) == 0) {
		iapp_set_program_status(1);
	}
	while(1) {
		int command = iapp_get_command();
		if(command == IAPP_GET) {
		} else if(( (command == IAPP_RUN) || cut_through) && iapp_get_program_status()) {
			SCB->VTOR = (0x80000 << 0);
			fn();
		}
	}

	return 0;
}
