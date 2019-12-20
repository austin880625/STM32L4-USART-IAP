#include "inc/stm32l476xx.h"

// use this pragma at handlers
//#pragma thumb

void delay() {
	int cnt = 1000000;
	while(cnt--);
}

int main() {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN;

	GPIOA->MODER = (GPIOA->MODER & 0xFFFFF3FF) | 0x400;
	GPIOA->OSPEEDR = 0x800;
	while(1) {
		GPIOA->ODR ^= (1<<5);
		delay();
		GPIOA->ODR ^= (1<<5);
		delay();
	}
	return 0;
}
