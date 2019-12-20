#include "inc/stm32l476xx.h"

// use this pragma at handlers
//#pragma thumb

uint8_t usart_buf[8192];

#pragma thumb
void USART1_Handler() {
	NVIC->ICPR[1] = 0x20;
	uint32_t checksum = 0;
	for(int i=4; i; i--) {
		while(!(USART1->ISR & USART_ISR_RXNE));
		uint8_t c = USART1->RDR;
		checksum <<= 8;
		checksum |= (uint32_t)c;
	}
	uint32_t wait_cnt = 0, ptr = 0, checksum_self = 0;
	while(1) {
		if(USART1->ISR & USART_ISR_RXNE) {
			wait_cnt = 0;
			usart_buf[ptr++] = USART1->RDR;
			if(ptr & 0x03 == 0) {
				uint32_t delta = 0;
				for(int i=4; i; i--) {
					delta <<= 8;
					delta |= (uint32_t)usart_buf[ptr-i];
				}
				checksum_self = delta;
			}
			while(!(USART1->ISR & USART_ISR_TXE));
			USART1->TDR = (uint8_t)(ptr & 0xFF);
		}
		//if(ptr > 1024)break;
		/*
		wait_cnt++;
		if(wait_cnt > 8) {
			break;
		}
		*/
	}
	while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = (uint8_t)(checksum_self & 0xFF);
	checksum_self >>= 8;
	while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = (uint8_t)(checksum_self & 0xFF);
	checksum_self >>= 8;
	while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = (uint8_t)(checksum_self & 0xFF);
	checksum_self >>= 8;
	while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = (uint8_t)(checksum_self & 0xFF);
	/*
	if(checksum == 0) {
		USART1->TDR = '\0';
	} else {
		USART1->TDR = 0x01;
	}
	*/
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
	USART_init();
	NVIC_init();
	return 0;
}
