#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "math.h"			
#include "stdio.h"
#include "stm32f10x_flash.h"
#include "stdlib.h"
#include "string.h"
#include "wdg.h"
#include "stm32f10x_it.h" 
extern char  RxBuffer[100],RxCounter;
extern unsigned char uart1_getok;
extern char RxCounter1,RxBuffer1[100];
extern unsigned char Timeout,restflag;



 int main(void)
 {	
    delay_init();	    	     //延时函数初始化	  
    NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    uart_init(115200);       //串口1初始化，可连接PC进行打印模块返回数据	 
		USART2_Init(9600);       //串口2初始化，可连接NB模块发送AT指令连接阿里云
		DMA_Configuration();     //DMA初始化
		ADC1_Configuration();    //ADC初始化
		ADC_SoftwareStartConvCmd(ADC1,ENABLE);//开启ADC1
		while(1)
		{
			atk_bc26_connect();    //激活NB模块连接到云端
			atk_bc26_send();       //NB模块发送数据到云端
			delay_ms(1000);
			
		}
	 

 }






