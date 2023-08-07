#include "delay.h"
#include "usart2.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	   
#include "stm32f10x.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_tim.h"
#include "bsp_dht11.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "Lcd_Driver.h"
#include "GUI.h"
#include "stm32f10x_it.h" 
extern uint8_t Usart_Send_Flag;

//���ڷ��ͻ����� 	
__align(8) u8 USART2_TX_BUF[USART2_MAX_SEND_LEN]; 	//���ͻ���,���USART2_MAX_SEND_LEN�ֽ�
#ifdef USART2_RX_EN   								//���ʹ���˽���   	  
//���ڽ��ջ����� 	
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//���ջ���,���USART2_MAX_RECV_LEN���ֽ�.


uint8_t temp,humi;
extern uint8_t p1,p2,p3;
char *cmd1="AT+QMTCFG=\"ALIAUTH\",0,\"gwvcku0pZq0\",\"region1\",\"5667062f7d1fbb2400ca2565fbece09f\"";//����MQTT�汾
char *cmd2="AT+QMTOPEN=0,\"iot-as-mqtt.cn-shanghai.aliyuncs.com\",1883";//����MQTT,������IP���˿ں�
char *cmd3="AT+QMTCONN=0,\"region1\"";//�ͻ������û���
char *cmd4="AT+QMTPUB=0,0,0,0,\"/sys/gwvcku0pZq0/region1/thing/event/property/post\"";//��������
char *cmd5="{params:{ RelativeHumidity:58}}";
char *cmd6="1A";
char cmd7=0x1A;
//ͨ���жϽ�������2���ַ�֮���ʱ������10ms�������ǲ���һ������������.
//���2���ַ����ռ������10ms,����Ϊ����1����������.Ҳ���ǳ���10msû�н��յ�
//�κ�����,���ʾ�˴ν������.
//���յ�������״̬
//[15]:0,û�н��յ�����;1,���յ���һ������.
//[14:0]:���յ������ݳ���
u16 USART2_RX_STA=0;   	 
void USART2_IRQHandler(void)
{
	u8 res;	    
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//���յ�����
	{	 
 
	res =USART_ReceiveData(USART2);		
		if(USART2_RX_STA<USART2_MAX_RECV_LEN)		//�����Խ�������
		{
			TIM_SetCounter(TIM4,0);//���������        				 
			if(USART2_RX_STA==0)TIM4_Set(1);	 	//ʹ�ܶ�ʱ��4���ж� 
			USART2_RX_BUF[USART2_RX_STA++]=res;		//��¼���յ���ֵ	 
		}else 
		{
			USART2_RX_STA|=1<<15;					//ǿ�Ʊ�ǽ������
		} 
	}  											 
}   
//��ʼ��IO ����2
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������	  
void USART2_Init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

 	USART_DeInit(USART2);  //��λ����2
		 //USART2_TX   PA.2
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA2
   
    //USART2_RX	  PA.3
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA3
	
	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  
	USART_Init(USART2, &USART_InitStructure); //��ʼ������2
  
	//����������
  //	USART2->BRR=(pclk1*1000000)/(bound);// ����������	 
	//USART2->CR1|=0X200C;  	//1λֹͣ,��У��λ.
	USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE);  	//ʹ�ܴ���2��DMA����
	UART_DMA_Config(DMA1_Channel7,(u32)&USART2->DR,(u32)USART2_TX_BUF);//DMA1ͨ��7,����Ϊ����2,�洢��ΪUSART2_TX_BUF 
	USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 
	
#ifdef USART2_RX_EN		  	//���ʹ���˽���
	//ʹ�ܽ����ж�
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�   
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	TIM4_Init(999,7199);		//100ms�ж�
	USART2_RX_STA=0;		//����
	TIM4_Set(0);			//�رն�ʱ��4
#endif	 	

}
//����2,printf ����
//ȷ��һ�η������ݲ�����USART2_MAX_SEND_LEN�ֽ�
void u2_printf(char* fmt,...)  
{  
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char*)USART2_TX_BUF,fmt,ap);
	va_end(ap);
	while(DMA_GetCurrDataCounter(DMA1_Channel7)!=0);	//�ȴ�ͨ��7�������   
	UART_DMA_Enable(DMA1_Channel7,strlen((const char*)USART2_TX_BUF)); 	//ͨ��dma���ͳ�ȥ
}
//��ʱ��4�жϷ������		    
void TIM4_IRQHandler(void)
{ 	
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)//�Ǹ����ж�
	{	 			   
		USART2_RX_STA|=1<<15;	//��ǽ������
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //���TIMx�����жϱ�־    
		TIM4_Set(0);			//�ر�TIM4  
	}	    
}
//����TIM4�Ŀ���
//sta:0���ر�;1,����;
void TIM4_Set(u8 sta)
{
	if(sta)
	{
       
		TIM_SetCounter(TIM4,0);//���������
		TIM_Cmd(TIM4, ENABLE);  //ʹ��TIMx	
	}else TIM_Cmd(TIM4, DISABLE);//�رն�ʱ��4	   
}
//ͨ�ö�ʱ���жϳ�ʼ��
//����ʼ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��		 
void TIM4_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //ʱ��ʹ��//TIM4ʱ��ʹ��    
	
	//��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM4�ж�,��������ж�

	 	  
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
}
#endif		 
///////////////////////////////////////USART2 DMA�������ò���//////////////////////////////////	   		    
//DMA1�ĸ�ͨ������
//����Ĵ�����ʽ�ǹ̶���,���Ҫ���ݲ�ͬ��������޸�
//�Ӵ洢��->����ģʽ/8λ���ݿ��/�洢������ģʽ
//DMA_CHx:DMAͨ��CHx
//cpar:�����ַ
//cmar:�洢����ַ    
void UART_DMA_Config(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar)
{
	DMA_InitTypeDef DMA_InitStructure;
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMA����
  DMA_DeInit(DMA_CHx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ
	DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA����ADC����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //���ݴ��䷽�򣬴��ڴ��ȡ���͵�����
	DMA_InitStructure.DMA_BufferSize = 0;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //��������������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA_CHx, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���	
} 
//����һ��DMA����
void UART_DMA_Enable(DMA_Channel_TypeDef*DMA_CHx,u8 len)
{
	DMA_Cmd(DMA_CHx, DISABLE );  //�ر� ָʾ��ͨ��        
	DMA_SetCurrDataCounter(DMA_CHx,len);//DMAͨ����DMA����Ĵ�С	
	DMA_Cmd(DMA_CHx, ENABLE);           //����DMA����
}	   
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 									 


//usmart֧�ֲ���
//���յ���ATָ��Ӧ�����ݷ��ظ����Դ���
//mode:0,������USART2_RX_STA;
//     1,����USART2_RX_STA;


u8* atk_nb_check_cmd(char *str)
{
	
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//��NBIOT��������
//cmd:���͵������ַ���
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 atk_nb_send_cmd(char *cmd,char *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	u2_printf("%s\r\n",cmd);	//��������
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				printf("wait check\n");
				if(atk_nb_check_cmd(ack))
				{
					break;//�õ���Ч���� 
				}
				USART2_RX_STA=0;
				memset(USART2_RX_BUF,0,sizeof(USART2_RX_BUF));
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 

void USART2_Send_Data(char *buf,u16 len)
 {
      u16 t;
      for(t=0;t<len;t++)        
      {           
          while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);      
          USART_SendData(USART2,buf[t]);
      }     
      while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);          
 }

//ͨ������2����ATָ���NBģ�飬���ӵ��Լ���İ�����ƽ̨
//ATI�鿴��ǰģ��汾
//AT+CIMI���SIM���Ƿ�����ɹ�
//AT+CGSN=1��ȡIMEIΨһ���к�
//AT+CGATT?���ģ���ܷ�����
void atk_bc26_connect(void)
{
	
	while(atk_nb_send_cmd("ATI","OK",20))
	{
		delay_ms(800);
	}
	printf("ATIͨѶ�ɹ�\r\n");
	delay_ms(100);
	while(atk_nb_send_cmd("AT+CIMI","OK",20))
	{
		delay_ms(800);	
	}
	printf("CIMIͨѶ�ɹ�\r\n");

	while(atk_nb_send_cmd("AT+CGSN=1","OK",100))
	{
		delay_ms(800);
	}

	
	while(atk_nb_send_cmd("AT+CGATT?","+CGATT",100))
	{
		delay_ms(800);
	}
	printf("ģ�鼤��\r\n");		
	

	while(atk_nb_send_cmd(cmd1,"OK",100))
	{
		delay_ms(1000);
	}
	printf("cmd1ͨѶ�ɹ�\r\n");	
	
	delay_ms(1000);
	while(atk_nb_send_cmd(cmd2,"OK",100))
	{
		delay_ms(1000);
	}
	printf("cmd2ͨѶ�ɹ�\r\n");		
	delay_ms(1000);

	while(atk_nb_send_cmd(cmd3,"OK",100))
	{
		delay_ms(1000);
	}
	delay_ms(1000);
	printf("cmd3ͨѶ�ɹ�\r\n");
}

//NBģ�����Ӱ����ƺ�������
//����ͬ����ʾ����ʾ����
void atk_bc26_send(void)
{
		while(atk_nb_send_cmd(cmd4,">",100))//���WIFIģ���Ƿ�����
		{
			delay_ms(1000);
		}
		delay_ms(1000);
		printf("cmd4ͨѶ�ɹ�\r\n");	

		Lcd_Init();
		LCD_LED_SET;//ͨ��IO���Ʊ�����		
		Lcd_Clear(WHITE);
		Gui_DrawFont_GBK16(15,0,BLUE,YELLOW,"AIR QUALITY");
		DHT11_Read_Data(&temp,&humi);
		char t[200];
		char h[200];
		char s[200];
		char c[200];
		char p[200];
    if(Usart_Send_Flag == 1)
		{
			sprintf(s,"%d",p1);
			sprintf(c,"%d",p2);	
			sprintf(p,"%d",p3);
			sprintf(t,"%d",temp);
			sprintf(h,"%d",humi);
			Usart_Send_Flag = 0;	
			
			Gui_DrawFont_GBK16(0,20,BLUE,WHITE,"Sulfide:");
			Gui_DrawFont_GBK16(70,20,BLUE,WHITE,s);
			Gui_DrawFont_GBK16(94,20,BLUE,WHITE,"ppm");
			Gui_DrawFont_GBK16(0,40,BLUE,WHITE,"CO2:");
			Gui_DrawFont_GBK16(70,40,BLUE,WHITE,c);
			Gui_DrawFont_GBK16(94,40,BLUE,WHITE,"ppm");
			Gui_DrawFont_GBK16(0,60,BLUE,WHITE,"Smog:");
			Gui_DrawFont_GBK16(70,60,BLUE,WHITE,p);
			Gui_DrawFont_GBK16(94,60,BLUE,WHITE,"ppm");
			Gui_DrawFont_GBK16(0,80,BLUE,WHITE,"Temperature:");
			Gui_DrawFont_GBK16(94,80,BLUE,WHITE,t);
			Gui_DrawFont_GBK16(114,80,BLUE,WHITE,"C");
			Gui_DrawFont_GBK16(0,100,BLUE,WHITE,"Humidity:");
			Gui_DrawFont_GBK16(70,100,BLUE,WHITE,h);
			Gui_DrawFont_GBK16(90,100,BLUE,WHITE,"%");
			char json[]="{params:{RelativeHumidity:%d,Temperature:%d,sulfide:%d,CO2:%d,smog:%d}}";	 //  
		//	char json[]="{params:{RelativeHumidity:%d,Temperature:%d}}";	
			char t_json[200];
			
			delay_ms(1200);
			
			sprintf(t_json, json, humi, temp,p1,p2,p3);
			
			USART2_Send_Data(t_json,strlen(t_json));
			delay_ms(100);
			USART2_Send_Data(&cmd7,1);
			delay_ms(1000);
			Usart_Send_Flag = 0;
		}
}











