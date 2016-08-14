#include "time.h"
#include "W5200.h"
#include "led.h"
#include "stm32f4xx_gpio.h"

extern u8 RJ45_2_Connect;     //����״̬

// TIM3->CNT��¼��ǰ��ֵ
/********************����˵��**********************/
//����˵��    u16 arr �Զ���װֵ 0~65535  u16 psc Ԥ��Ƶϵ��  0~65535
//��ʱ��ʱ��Ƶ�ʣ�84M
void TIM3_Init(u16 arr,u16 psc)   
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///ʹ��TIM3ʱ��
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 

	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//��ʼ��TIM3
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
	TIM_Cmd(TIM3,ENABLE ); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=DISABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}
//      �жϺ���
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
	{
	  GPIO_ToggleBits(GPIOB,GPIO_Pin_9);  //DS1��ת
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
}
/**
*@brief		��ʱ1S.������ʾ����ָʾ
*/
void TIM4_Init(void)   
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  ///ʹ��TIM3ʱ��
	
  TIM_TimeBaseInitStructure.TIM_Period = 10000-1; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=8400-1;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 

	
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);//��ʼ��TIM3
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
	TIM_Cmd(TIM4,ENABLE ); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}
//      �жϺ���
void TIM4_IRQHandler(void)
{
	u8 Flag;  //��ȡ����״̬
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)==SET) //����ж�
	{
	  LED1=!LED1;
		Flag=RJ45_2_Read_Register(Sn_SR(0));
		if(Flag==SOCK_CLOSED)  //���ڹرգ�ָʾ�Ʋ���˸�����³�ʼ������
		{
			LED4 =0; //�ر�����ָʾ��
			RJ45_2_Init();
	    RJ45_2_TCP_ServiceInit();  //���¿�������
			RJ45_2_Connect=0;
		}
		if((Flag==SOCK_INIT)||(Flag==SOCK_LISTEN))  //��ʼ���׶λ��߼����׶�
		{
			RJ45_2_Connect=0;
		}
		if(Flag==SOCK_ESTABLISHED) //���罨������
		{
			LED4 =1;
		}
	}
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);  //����жϱ�־λ
}