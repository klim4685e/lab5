#include "stm32f401xc.h"
#include <stdint.h>

#define TIM_PRESCALER	4000
#define ARR_MAX		200
#define CCR1_MAX	200

typedef enum
{
	STATE_PWM_RUNNING,
	STATE_PFM_RUNNING,
	STATE_PWM_REQUEST,
	STATE_PFM_REQUEST
}state_t;

uint8_t dir;
state_t mode;
void EXTI0_Init(void);
void LED_Init(void);
void TIM_Init(void);
int main(void)
{
	EXTI0_Init();
	LED_Init();
	TIM_Init();
	mode = STATE_PFM_REQUEST;


	while(1)
	{
		if(mode == STATE_PFM_REQUEST)
		{
			TIM10->CR1 &= ~TIM_CR1_CEN;
			for(uint32_t i = 0; i < 500000;i++)
				__asm("nop");
			mode = STATE_PFM_RUNNING;
			TIM10->ARR = 1;
			TIM10->CCR1 = 1;
			TIM10->CNT = 0;
			TIM10->CR1 |= TIM_CR1_CEN;
		}
		else if(mode == STATE_PWM_REQUEST)
		{
			TIM10->CR1 &= ~TIM_CR1_CEN;
			for(uint32_t i = 0; i < 500000;i++)
				__asm("nop");
			mode = STATE_PWM_RUNNING;
			TIM10->CCR1 = 1;
			TIM10->ARR = ARR_MAX-1;
			TIM10->CNT = 0;
			TIM10->CR1 |= TIM_CR1_CEN;
		}
	}
	return 0;
}

void EXTI0_IRQHandler(void)
{
	switch(mode)
	{
		case STATE_PFM_RUNNING:
			mode = STATE_PWM_REQUEST;
			break;
		case STATE_PWM_RUNNING:
			mode = STATE_PFM_REQUEST;
			break;
		default:
			break;
	}


	EXTI->PR = EXTI_PR_PR0;
	asm("nop");
	  asm("nop");
	  asm("nop");
	  asm("nop");
	  asm("nop");
}
void TIM1_UP_TIM10_IRQHandler(void)
{
	if(TIM10->SR & TIM_SR_CC1IF)
	{
		switch (mode)
		{
			case STATE_PWM_RUNNING:
				GPIOC->ODR ^= GPIO_ODR_OD13;
				break;
			default:
				break;
		}

		TIM10->SR &= ~TIM_SR_CC1IF;
	}

	else if(TIM10->SR & TIM_SR_UIF)
	{
		switch(mode)
		{
			case STATE_PWM_RUNNING:
				GPIOC->ODR |= GPIO_ODR_OD13;
				if(TIM10->CCR1 > CCR1_MAX)
					dir = 1;
				else if(TIM10->CCR1 < 2)
					dir = 0;

				if(dir == 0)
				{
					TIM10->CCR1++;
				}
				else
					TIM10->CCR1--;


				TIM10->CR1 |= TIM_CR1_CEN;
				break;

			case STATE_PFM_RUNNING:
				GPIOC->ODR ^= GPIO_ODR_OD13;
				if(TIM10->ARR > ARR_MAX)
					TIM10->ARR = 1;
				else
					TIM10->ARR++;

				TIM10->CR1 |= TIM_CR1_CEN;
				break;
			default:
				break;
		}
		TIM10->SR &= ~TIM_SR_UIF;
	}

}


void EXTI0_Init(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	GPIOA->PUPDR |= GPIO_PUPDR_PUPD0_0;
	SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI0_PA;
	EXTI->RTSR |= EXTI_RTSR_TR0;
	EXTI->FTSR |= EXTI_FTSR_TR0;
	EXTI->IMR |= EXTI_IMR_IM0;

	NVIC_EnableIRQ(EXTI0_IRQn);
}
void LED_Init(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	GPIOC->MODER |= GPIO_MODER_MODE13_0;
}
void TIM_Init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
	TIM10->PSC = TIM_PRESCALER-1;
	TIM10->CCMR1 |= 0x07 << 4; //PWM Mode 1
	TIM10->DIER |= TIM_DIER_UIE | TIM_DIER_CC1IE;
	TIM10->CR1 |= TIM_CR1_OPM;
	NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
}
