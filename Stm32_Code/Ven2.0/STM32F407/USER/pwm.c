#include "pwm.h"

/******************************************************************************/
/*                              PWM初始化                                     */
/******************************************************************************/

void pwm_init(u32 psc, u32 arr)
{		 					 	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	///////////////////////////////////////////////////////////////////////
	////                        GPIO配置                               ////
	///////////////////////////////////////////////////////////////////////
	   
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 	//使能PORTA时钟	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_TIM14); //GPIOA7复用为定时器14
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;           //GPIOA7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure);              //初始化PA7

	///////////////////////////////////////////////////////////////////////
	////                         PWM配置                               ////
	///////////////////////////////////////////////////////////////////////
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14,ENABLE);  	//TIM14时钟使能 
	
	TIM_TimeBaseStructure.TIM_Prescaler=psc-1;  //定时器分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseStructure.TIM_Period=arr-1;   //自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision=0; 
	
	TIM_TimeBaseInit(TIM14,&TIM_TimeBaseStructure);//初始化定时器14
		 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式1
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OCInitStructure.TIM_Pulse = (arr-1)/2; //(arr-1)/2之后输出翻转
	TIM_OC1Init(TIM14, &TIM_OCInitStructure);  //根据指定的参数初始化外设TIM14OC1

	TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Enable);  //使能TIM14在CCR1上的预装载寄存器
 
  TIM_ARRPreloadConfig(TIM14,ENABLE);//ARPE使能 
	
	TIM_Cmd(TIM14, ENABLE);  //使能TIM14

} 
