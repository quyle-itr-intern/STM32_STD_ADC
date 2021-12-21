#include "stm32f10x.h"
#include "stdio.h"
#include "stm32f10x_it.h"
#include "stm32f10x_adc.h"
#include "string.h"
#include "stdlib.h"
#include "stdint.h"
#include "math.h"

void TIM1_Config(void);
void ADC_Config(void);
void DMA_Config(uint32_t pStartAddress, uint32_t pDestination, uint32_t u32NumberDataTranfer);
#define ADC1_DR_Address     ((uint32_t) 0x4001244C)
#define ADC_CHANNEL 2
static volatile uint16_t AdcValues[ADC_CHANNEL];
static uint32_t giatri = 0;
static volatile uint16_t ADC_PA0 = 0;
static volatile uint16_t ADC_PA1 = 0;

void Delay_ms(uint32_t t);

int main(void)
{
	TIM1_Config();
	DMA_Config((uint32_t) ADC1_DR_Address, (uint32_t) AdcValues, ADC_CHANNEL);
	ADC_Config();
	
	while(1)
	{
		Delay_ms(1000);
		giatri++;
		ADC_PA0 = AdcValues[0];
		ADC_PA1 = AdcValues[1];
	}
}

void ADC_Config(void)
{
	GPIO_InitTypeDef GPIO;
	ADC_InitTypeDef ADC_1;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO.GPIO_Mode = GPIO_Mode_AIN;
	GPIO.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO);
	
	/* ADC1 configuration ------------------------------------------------------*/
  ADC_1.ADC_Mode = ADC_Mode_Independent;
  ADC_1.ADC_ScanConvMode = DISABLE;
  ADC_1.ADC_ContinuousConvMode = ENABLE;
  ADC_1.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_1.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_1.ADC_NbrOfChannel = ADC_CHANNEL;
  ADC_Init(ADC1, &ADC_1);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);// PA0
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_239Cycles5);// PA1
	
	/* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  /* Enable ADC1 reset calibration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));
  /* Start ADC1 calibration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void DMA_Config(uint32_t pStartAddress, uint32_t pDestination, uint32_t u32NumberDataTranfer)
{
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA1_Channel1->CPAR = (uint32_t) pStartAddress;
	DMA1_Channel1->CMAR = (uint32_t) pDestination;
	DMA1_Channel1->CNDTR = u32NumberDataTranfer;
	DMA1_Channel1->CCR |= 0x25A0;
	DMA1_Channel1->CCR |= 0x01;
}

void Delay_ms(uint32_t t)
{
	while(t)
	{
		TIM_SetCounter(TIM1, 0);
		while(TIM_GetCounter(TIM1) < 1000);
		t--;
	}
}

void TIM1_Config(void)
{
	TIM_TimeBaseInitTypeDef  TIMER_1;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	TIMER_1.TIM_CounterMode = TIM_CounterMode_Up;  
	TIMER_1.TIM_Period = 0xFFFF;          
  TIMER_1.TIM_Prescaler = 72;
	
	TIM_TimeBaseInit(TIM1, &TIMER_1);

	TIM_Cmd(TIM1, ENABLE);
}
