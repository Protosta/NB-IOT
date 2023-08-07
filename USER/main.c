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
    delay_init();	    	     //��ʱ������ʼ��	  
    NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    uart_init(115200);       //����1��ʼ����������PC���д�ӡģ�鷵������	 
		USART2_Init(9600);       //����2��ʼ����������NBģ�鷢��ATָ�����Ӱ�����
		DMA_Configuration();     //DMA��ʼ��
		ADC1_Configuration();    //ADC��ʼ��
		ADC_SoftwareStartConvCmd(ADC1,ENABLE);//����ADC1
		while(1)
		{
			atk_bc26_connect();    //����NBģ�����ӵ��ƶ�
			atk_bc26_send();       //NBģ�鷢�����ݵ��ƶ�
			delay_ms(1000);
			
		}
	 

 }






