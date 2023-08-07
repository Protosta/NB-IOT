/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h" 
#include "usart.h"
#include "math.h"
#include "DMA.h"

#define         DC_GAIN                      (8.5)   //模块放大倍数
#define         ZERO_POINT_VOLTAGE           (0.296) //400PPM二氧化碳气体中的电压值
#define         REACTION_VOLTGAE             (0.027) //由400ppm二氧化碳其中放入1000ppm二氧化碳气体中的压降值

uint16_t Air_Quality=0;
uint16_t CO2_Concentration=0;
uint16_t smoke_Concentration=0;
uint8_t Usart_Send_Flag=0;
uint8_t Sent_State=0;
uint8_t p1,p2,p3;
float U1,U2,U3;
float    GASCurve[3]  =  {2.602,ZERO_POINT_VOLTAGE,(REACTION_VOLTGAE/(2.602-3))};//（0.296-0.269）/（lg400-lg1000）

void NMI_Handler(void)
{
}
 
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}
 
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

 
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}
 
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}
 
void SVC_Handler(void)
{
}
 
void DebugMon_Handler(void)
{
}
 
void PendSV_Handler(void)
{
}
 
void SysTick_Handler(void)
{
}

//DMA接收中断完成后触发DMA中断接收AD值并用公式转换成气体浓度
void DMA1_Channel1_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC1)!=RESET) 
	{   
		DMA_ClearITPendingBit(DMA1_IT_TC1);
		
		if(Usart_Send_Flag == 0)
		{
			Air_Quality=ADC_ConvertedValue[0];		
			CO2_Concentration=ADC_ConvertedValue[1];		
			smoke_Concentration=ADC_ConvertedValue[2];
			U1= Air_Quality*3.3/4096;
			U2= CO2_Concentration*5/4096;
			U3= smoke_Concentration*3.3/4096;
			p1 = pow((3.4880*10*U1)/(3.3-U1),(1.0/0.3203));
			p2 = pow(10, ((U2/DC_GAIN)-GASCurve[1])/GASCurve[2]+GASCurve[0]);
			p3=pow(11.5428*35.9*U3/(25.5-5.1*U3),0.6549);
//			printf("Air_Quality:%d\n",p1);
//			printf("CO2_Concentration:%d\n",p2);
//			printf("smoke_Concentration:%d\n",p3);
			Usart_Send_Flag = 1;
//			Sent_State=1;
		}
	} 
}
/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
