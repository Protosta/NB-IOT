#ifndef __MYTIM_H
#define __MYTIM_H	 

#include "stm32f10x.h"

void Tim1_Init(int arr,int psc);
void TIM1_UP_IRQHandler (void);
void TIM2_Init(u16 arr,u16 psc);
void TIM2_Set(unsigned char sta);

#endif
