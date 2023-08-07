#ifndef __DHT11_H__
#define	__DHT11_H__
#include "stm32f10x.h"
#include "delay.h"

#define 	DHT_Direction_I  	{GPIOA->CRL &= 0XFFFFFFF0; GPIOA->CRL |= 8;}
#define 	DHT_Direction_O 	{GPIOA->CRL &= 0XFFFFFFF0; GPIOA->CRL |= 3;}
/*	IO Definitions	*/
#define		O_DHT_DAT		PAout(0)
#define		I_DHT_DAT		PAin(0)
#define		DHT_DAT_Pin		GPIO_Pin_0
#define    	DHTPORT         GPIOA
#define     DHTCLKLINE      RCC_APB2Periph_GPIOA

extern void DHT11Configuration(void);
extern uint8_t DHT11_IsOnline(void);
extern uint8_t DHT11_Read_Data(uint8_t *temp,uint8_t *humi);
#endif

