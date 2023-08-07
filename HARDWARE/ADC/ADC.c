#include "ADC.h"
#include "stm32f10x.h"

void ADC1_Configuration()
{
		ADC_InitTypeDef  ADC_InitStructure;
	
		GPIO_ADC1_Configuration();	
	
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	
		RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
		ADC_DeInit(ADC1);
	
		ADC_StructInit(&ADC_InitStructure);

		ADC_InitStructure.ADC_Mode=ADC_Mode_Independent;
		ADC_InitStructure.ADC_ScanConvMode=ENABLE;
		ADC_InitStructure.ADC_ContinuousConvMode=ENABLE;
		ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;
		ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NbrOfChannel=3;
		ADC_Init(ADC1, &ADC_InitStructure);

		ADC_RegularChannelConfig(ADC1,ADC_Channel_1,1,ADC_SampleTime_28Cycles5);
		ADC_RegularChannelConfig(ADC1,ADC_Channel_6,2,ADC_SampleTime_28Cycles5);
		ADC_RegularChannelConfig(ADC1,ADC_Channel_9,3,ADC_SampleTime_28Cycles5);	

		ADC_DMACmd(ADC1, ENABLE); 
	
		ADC_Cmd(ADC1, ENABLE);
		
		ADC_ResetCalibration(ADC1);

		while(ADC_GetResetCalibrationStatus(ADC1));

		ADC_StartCalibration(ADC1);
		
		while(ADC_GetCalibrationStatus(ADC1));
}
