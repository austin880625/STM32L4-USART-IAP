#include "inc/stm32l476xx.h"

// use this pragma at handlers
#pragma thumb
void SysTick_Handler() {
	GPIOA->ODR = GPIOA->ODR ^ (1<<5);
}

void GPIO_init() {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	GPIOA->MODER = (GPIOA->MODER & 0xFFFFF3FF) | 0x400;
	GPIOA->OSPEEDR = 0x800;
}

int main() {
	GPIO_init();
	SysTick->LOAD = 600000;
    SysTick->CTRL = 3;
	return 0;
}
