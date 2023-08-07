#include "mytim.h"
#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stdio.h"
#include "string.h"
#include "led.h"


//extern volatile   unsigned char ledState;

extern volatile   unsigned char crlFlag;
extern u8 USART2_RX_BUF[200];     //接收缓冲,最大USART_REC_LEN个字节.

extern u16 USART2_RX_STA;       //接收状态标记
void   Usart2_Recv_Task(void)
{
    u16 rlen = 0;   
}



//extern unsigned int Heart_Pack;  //用于定时器TIM1自加计数，用于满足设定自加数值时发送EDP心跳包的标志位


void Tim1_Init(int arr,int psc)
{ 
	TIM_TimeBaseInitTypeDef TIM_Structure;               //定义定时器结构体变量
	NVIC_InitTypeDef NVIC_TIM;                           //定义中断嵌套结构体变量
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);  //打开定时器时钟

	TIM_Structure.TIM_Period = (arr-1) ;         //设置自动重装载寄存器周期值  溢出时间TimeOut= (arr)*(psc)/Tic    单位为us
	TIM_Structure.TIM_Prescaler = (psc-1);       //设置预分频值     
	TIM_Structure.TIM_CounterMode = TIM_CounterMode_Up ;     //计数模式 上升计数
	TIM_Structure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频      Tic=72M/（TIM_ClockDivision+1）
	TIM_Structure.TIM_RepetitionCounter = 0; //重复计数的次数

	TIM_TimeBaseInit(TIM1,&TIM_Structure);   //初始化定时器1

	NVIC_TIM.NVIC_IRQChannel = TIM1_UP_IRQn;  //定时器1的向上计算通道
	NVIC_TIM.NVIC_IRQChannelCmd = ENABLE ;    //使能
	NVIC_TIM.NVIC_IRQChannelPreemptionPriority = 0 ;    //抢占优先级
	NVIC_TIM.NVIC_IRQChannelSubPriority = 0;            //响应优先级 

	NVIC_Init(&NVIC_TIM);                     //初始化结构体

	TIM_ClearFlag(TIM1,TIM_FLAG_Update);      //清空所有标志位  保证工作状态初始化 

	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);  //打开计时器

	TIM_Cmd(TIM1,ENABLE);      	  	 	       //打开TIM1
	
	 
}	

/*****定时器1中断响应函数*****/
/*   需要查询中断函数名字 请打开 startup_stm32f10x_hd.s启动文件查询对应的中断响应名字*/
/*   说明：TIM_ClearFlag()函数是清除该定时器的所有标志位  
          一个定时器的标志位包括很多如 TIM_IT_Update TIM_IT_CC1 TIM_IT_CC2 TIM_IT_CC3 等 
          所以我们用具体清空标志位函数TIM_ClearITPendingBit() 对某个标志位进行清除 如果在没有几个标志位同时工作 两函数的使用效果应该是一样的  */

void TIM1_UP_IRQHandler (void)                
{
   if(TIM_GetITStatus(TIM1,TIM_IT_Update) != RESET)   //如果中断标志被置1 证明有中断
	 {
		 
		 TIM_ClearITPendingBit(TIM1,TIM_IT_Update);    // 清空标志位，为下一次进入中断做准备	
         Usart2_Recv_Task();         
//		  printf("123\r\n");  //测试中断是否正常
	    //操作内容
	 }
  
}


//TIM2用于普通定时器检测
void TIM2_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(  //使能或者失能指定的TIM中断
		TIM2, //TIM2
		TIM_IT_Update ,
		ENABLE  //使能
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM2, ENABLE);  //使能TIMx外设
							 
}


extern void  SG90_Control(void);
static unsigned  char  count=0;

void TIM2_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //清除TIMx更新中断标志    
	}
}


//设置TIM4的开关
//sta:0，关闭;1,开启;
void TIM2_Set(unsigned char sta)
{
	if(sta)
	{      
		TIM_SetCounter(TIM2,0);//计数器清空
		TIM_Cmd(TIM2, ENABLE);  //使能TIMx	
	}else TIM_Cmd(TIM2, DISABLE);//关闭定时器4	   
}


