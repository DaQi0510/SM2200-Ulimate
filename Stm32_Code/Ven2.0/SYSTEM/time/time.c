#include "time.h"
#include "W5200.h"
#include "led.h"
#include "stm32f4xx_gpio.h"

extern u8 RJ45_1_Connect;     //连接状态
extern u8 RJ45_2_Connect;     //连接状态

// TIM3->CNT记录当前数值
/********************函数说明**********************/
//参数说明    u16 arr 自动重装值 0~65535  u16 psc 预分频系数  0~65535
//定时器时钟频率：84M
void TIM3_Init(u16 arr,u16 psc)   
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///使能TIM3时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 

	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//初始化TIM3
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM3,ENABLE ); //使能定时器3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=DISABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}
//      中断函数
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
	  GPIO_ToggleBits(GPIOB,GPIO_Pin_9);  //DS1翻转
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
}
/**
*@brief		定时1S.用于显示运行指示
*/
void TIM4_Init(void)   
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  ///使能TIM3时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = 10000-1; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=8400-1;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 

	
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);//初始化TIM3
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM4,ENABLE ); //使能定时器3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}
//      中断函数
void TIM4_IRQHandler(void)
{
	u8 Flag1,Flag2;  //读取网口状态
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)==SET) //溢出中断
	{
	  LED1=!LED1;
		Flag1=RJ45_1_Read_Register(Sn_SR(0));
		Flag2=RJ45_2_Read_Register(Sn_SR(0));
		//调试接口信号灯显示
		if((Flag2==SOCK_CLOSED)||(Flag2==SOCK_CLOSE_WAIT))  //网口关闭，指示灯不闪烁，重新初始化网口
		{
			LED4 =0; //关闭网口指示灯
			RJ45_2_Init();
	    RJ45_2_TCP_ServiceInit();  //重新开启网口
			RJ45_2_Connect=0;
		}
		if((Flag2==SOCK_INIT)||(Flag2==SOCK_LISTEN))  //初始化阶段或者监听阶段
		{
			RJ45_2_Connect=0;
		}
		if(Flag2==SOCK_ESTABLISHED) //网络建立连接
		{
			LED4 =1;
		}
		else
		{
			LED4 =0;
		}
		//网口信号灯显示
		if((Flag1==SOCK_CLOSED)||(Flag1==SOCK_CLOSE_WAIT))  //网口关闭，指示灯不闪烁，重新初始化网口
		{
			LED5 =0; //关闭网口指示灯
			RJ45_1_Connect=0;
		}
		if((Flag1==SOCK_INIT)||(Flag1==SOCK_LISTEN))  //初始化阶段或者监听阶段
		{
			RJ45_1_Connect=0;
		}
		if(Flag1==SOCK_ESTABLISHED) //网络建立连接
		{
			LED5 =1;
		}
		else
		{
			LED5 =0;
		}
	}
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);  //清除中断标志位
}