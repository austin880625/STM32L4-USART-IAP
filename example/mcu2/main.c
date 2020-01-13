#include "inc/stm32l476xx.h"
#define TIME_SEC 12.70

// use this pragma at handlers
//#pragma thumb

extern void max7219_send(int addr, int data);

void GPIO_init() {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN;
	GPIOB->MODER = (GPIOB->MODER & 0xFFF0003F) | 0x540;
	GPIOB->OSPEEDR = 0xA80;
	GPIOB->PUPDR = 0xAA100;

	GPIOC->MODER = (GPIOC->MODER & 0xFFFC03FF) | 0x15400;
	GPIOC->OSPEEDR = 0x2A800;
}

void max7219_init() {
	max7219_send(0x0C, 0x01);
	max7219_send(0x0B, 0x07);
	max7219_send(0x09, 0xFF);
	max7219_send(0x0A, 0x08);
}

void timer_init(TIM_TypeDef *timer) {
	RCC->APB1ENR1 |= 0x01;
	timer->PSC = 4000;
	timer->ARR = TIME_SEC * 1000;
	timer->CR1 = 0x09;
}

void timer_start(TIM_TypeDef *timer) {
	timer->EGR |= 0x01;
	timer->CR1 |= 0x01;
}

void display(int val) {
	max7219_send(0x01, val%10);
	val /= 10;
	max7219_send(0x02, val%10);
	val /= 10;
	max7219_send(0x03, 128 | val%10);
	val /= 10;
	for(int p=4; p<=8; p++) {
		if(val) {
			max7219_send(p, val%10);
		} else {
			max7219_send(p, 0x0F);
		}
		val/=10;
	}
}

int main() {
	GPIO_init();
	max7219_init();
	timer_init(TIM2);
	timer_start(TIM2);
	TIM2->SR &= (0xFFFFFFFE);
	while(1) {
		display(TIM2->CNT / 10);
		if(TIM2->SR & 1) {
			display(TIME_SEC * 100);
			break;
		}
	}
	while(1);

	return 0;
}
