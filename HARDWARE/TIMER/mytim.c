#include "mytim.h"
#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stdio.h"
#include "string.h"
#include "led.h"


//extern volatile   unsigned char ledState;

extern volatile   unsigned char crlFlag;
extern u8 USART2_RX_BUF[200];     //���ջ���,���USART_REC_LEN���ֽ�.

extern u16 USART2_RX_STA;       //����״̬���
void   Usart2_Recv_Task(void)
{
    u16 rlen = 0;   
}



//extern unsigned int Heart_Pack;  //���ڶ�ʱ��TIM1�ԼӼ��������������趨�Լ���ֵʱ����EDP�������ı�־λ


void Tim1_Init(int arr,int psc)
{ 
	TIM_TimeBaseInitTypeDef TIM_Structure;               //���嶨ʱ���ṹ�����
	NVIC_InitTypeDef NVIC_TIM;                           //�����ж�Ƕ�׽ṹ�����
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);  //�򿪶�ʱ��ʱ��

	TIM_Structure.TIM_Period = (arr-1) ;         //�����Զ���װ�ؼĴ�������ֵ  ���ʱ��TimeOut= (arr)*(psc)/Tic    ��λΪus
	TIM_Structure.TIM_Prescaler = (psc-1);       //����Ԥ��Ƶֵ     
	TIM_Structure.TIM_CounterMode = TIM_CounterMode_Up ;     //����ģʽ ��������
	TIM_Structure.TIM_ClockDivision = TIM_CKD_DIV1;     //ʱ�ӷ�Ƶ      Tic=72M/��TIM_ClockDivision+1��
	TIM_Structure.TIM_RepetitionCounter = 0; //�ظ������Ĵ���

	TIM_TimeBaseInit(TIM1,&TIM_Structure);   //��ʼ����ʱ��1

	NVIC_TIM.NVIC_IRQChannel = TIM1_UP_IRQn;  //��ʱ��1�����ϼ���ͨ��
	NVIC_TIM.NVIC_IRQChannelCmd = ENABLE ;    //ʹ��
	NVIC_TIM.NVIC_IRQChannelPreemptionPriority = 0 ;    //��ռ���ȼ�
	NVIC_TIM.NVIC_IRQChannelSubPriority = 0;            //��Ӧ���ȼ� 

	NVIC_Init(&NVIC_TIM);                     //��ʼ���ṹ��

	TIM_ClearFlag(TIM1,TIM_FLAG_Update);      //������б�־λ  ��֤����״̬��ʼ�� 

	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);  //�򿪼�ʱ��

	TIM_Cmd(TIM1,ENABLE);      	  	 	       //��TIM1
	
	 
}	

/*****��ʱ��1�ж���Ӧ����*****/
/*   ��Ҫ��ѯ�жϺ������� ��� startup_stm32f10x_hd.s�����ļ���ѯ��Ӧ���ж���Ӧ����*/
/*   ˵����TIM_ClearFlag()����������ö�ʱ�������б�־λ  
          һ����ʱ���ı�־λ�����ܶ��� TIM_IT_Update TIM_IT_CC1 TIM_IT_CC2 TIM_IT_CC3 �� 
          ���������þ�����ձ�־λ����TIM_ClearITPendingBit() ��ĳ����־λ������� �����û�м�����־λͬʱ���� ��������ʹ��Ч��Ӧ����һ����  */

void TIM1_UP_IRQHandler (void)                
{
   if(TIM_GetITStatus(TIM1,TIM_IT_Update) != RESET)   //����жϱ�־����1 ֤�����ж�
	 {
		 
		 TIM_ClearITPendingBit(TIM1,TIM_IT_Update);    // ��ձ�־λ��Ϊ��һ�ν����ж���׼��	
         Usart2_Recv_Task();         
//		  printf("123\r\n");  //�����ж��Ƿ�����
	    //��������
	 }
  
}


//TIM2������ͨ��ʱ�����
void TIM2_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(  //ʹ�ܻ���ʧ��ָ����TIM�ж�
		TIM2, //TIM2
		TIM_IT_Update ,
		ENABLE  //ʹ��
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM2, ENABLE);  //ʹ��TIMx����
							 
}


extern void  SG90_Control(void);
static unsigned  char  count=0;

void TIM2_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //���TIMx�����жϱ�־    
	}
}


//����TIM4�Ŀ���
//sta:0���ر�;1,����;
void TIM2_Set(unsigned char sta)
{
	if(sta)
	{      
		TIM_SetCounter(TIM2,0);//���������
		TIM_Cmd(TIM2, ENABLE);  //ʹ��TIMx	
	}else TIM_Cmd(TIM2, DISABLE);//�رն�ʱ��4	   
}


