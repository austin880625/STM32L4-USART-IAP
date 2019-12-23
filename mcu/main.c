#include "inc/stm/stm32l476xx.h"
#include "inc/io/usart.h"

// use this pragma at handlers
//#pragma thumb


#define VTOR_BASE_ADDR ((uint32_t *)0x08080000U)

uint32_t program_size = 0;
uint8_t usart_buf[8192];
int program_ready = 0;

void flash_init() {
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;
}

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

#pragma thumb
void USART1_Handler() {
	NVIC->ICPR[1] = 0x20;
	NVIC->ICER[1] = 0x20;
	const uint32_t checksum = usart_receive_uint();
	uint32_t checksum_self = 0;
	uint32_t size = usart_receive_uint();
	checksum_self += size;
	uint32_t size_self = usart_receive(usart_buf + program_size, size);
	if(size == size_self) {
		checksum_self += usart_checksum(usart_buf + program_size, size);
	}
	usart_send_uint(checksum);
	if(checksum == checksum_self && size == size_self) {
		program_size += size;
		if(size == 0) {
			flash_write((uint32_t *)usart_buf, program_size);
			program_ready = 1;
		}
		usart_send_uint(0);
		NVIC->ISER[1] |= 0x20;
	} else {
		usart_send_uint(1);
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
}

int main() {
	GPIO_init();
	flash_init();
	USART_init();
	NVIC_init();
	program_ready = program_size = 0;
	while(!program_ready);
	uint32_t *new_vtor = VTOR_BASE_ADDR + 1;
	void (*fn)() = *((void (**)())(new_vtor));
	fn();
	while(1);

	return 0;
}
