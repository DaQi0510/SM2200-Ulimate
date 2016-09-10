#include "time.h"
#include "W5200.h"
#include "sm2200.h"
#include "led.h"
#include "stm32f4xx_gpio.h"

extern u8 RJ45_1_Connect;     //����״̬
extern u8 RJ45_2_Connect;     //����״̬
extern u8 RateFlag;  
extern volatile u8 SM2200ReadFlag; //��ǳ������ڶ�ȡ���� 1���ڶ�ȡ  0��ɶ�ȡ
extern volatile u32 ChannelReceive;         //��ǽ��յ�ͨ��

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
	u8 Flag1,Flag2,i;  //��ȡ����״̬
	u32 ChannelR;
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)==SET) //����ж�
	{
	  LED1=!LED1;
		ChannelR=OfdmXcvrRead(RECEIVE_STATUS, 3);
		if((ChannelR!=0)&&(SM2200ReadFlag==0))   //����������
		{
			for(i=0;i<200;i++);
		}
		if((ChannelR!=0)&&(SM2200ReadFlag==0))   //����������
		{
			SM2200_Receive();
			ChannelReceive=0;
			SM2200_Init();
	    LED3 =!LED3 ;
		}
		Flag1=RJ45_1_Read_Register(Sn_SR(0));
		Flag2=RJ45_2_Read_Register(Sn_SR(0));
		//���Խӿ��źŵ���ʾ
		if((Flag2==SOCK_CLOSED)||(Flag2==SOCK_CLOSE_WAIT))  //���ڹرգ�ָʾ�Ʋ���˸�����³�ʼ������
		{
			LED4 =0; //�ر�����ָʾ��
			RJ45_2_Init();
	    RJ45_2_TCP_ServiceInit();  //���¿�������
			RJ45_2_Connect=0;
		}
		if((Flag2==SOCK_INIT)||(Flag2==SOCK_LISTEN))  //��ʼ���׶λ��߼����׶�
		{
			RJ45_2_Connect=0;
		}
		if(Flag2==SOCK_ESTABLISHED) //���罨������
		{
			LED4 =1;
		}
		else
		{
			LED4 =0;
		}
		//�����źŵ���ʾ
		if((Flag1==SOCK_CLOSED)||(Flag1==SOCK_CLOSE_WAIT))  //���ڹرգ�ָʾ�Ʋ���˸�����³�ʼ������
		{
			LED5 =0; //�ر�����ָʾ��
			RJ45_1_Connect=0;
		}
		if((Flag1==SOCK_INIT)||(Flag1==SOCK_LISTEN))  //��ʼ���׶λ��߼����׶�
		{
			RJ45_1_Connect=0;
		}
		if(Flag1==SOCK_ESTABLISHED) //���罨������
		{
			LED5 =1;
		}
		else
		{
			LED5 =0;
		}
		TIM_ClearITPendingBit(TIM4,TIM_IT_Update);  //����жϱ�־λ
	}
	
}
/**
*@brief		��ʱ500ms.����ͳ��ͨ������
*/
void TIM5_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);  ///ʹ��TIM3ʱ��
	
  TIM_TimeBaseInitStructure.TIM_Period = 6000-1; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=8400-1;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 

	
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStructure);//��ʼ��TIM3
	
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
	TIM_Cmd(TIM5,ENABLE ); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM5_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
//�жϺ���
void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5,TIM_IT_Update)==SET) //����ж�
	{
		RateFlag=1;
		TIM_ClearITPendingBit(TIM5,TIM_IT_Update);  //����жϱ�־λ
	}
	
}
