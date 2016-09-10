#include "ads1271.h"
#include "pwm.h"
#include "delay.h"
#include "spi.h"
#include "dma.h"
#include "exti.h"

/******************************************************************************/
/*                             ADS1271初始化                                  */
/*											输入参数为0-->High-Speed mode													*/
/*										输入参数为1-->High-Resolution mode											*/
/*											输入参数为'z'-->Low-Power mode												*/
/******************************************************************************/

void ads1271_init(char mode)
{
	ads1271_mode(mode);
  pwm_init(1,3);//时钟频率 81/1/3 = 27MHz
	sync_config();//sync
	pdwn_calibration();//自校准
  sync();//同步
	spi1_init();//spi1初始化
	spi2_init();//spi2初始化
	dma2_init();//dma2初始化
	dma1_init();//dma1初始化
	exti_init();//中断初始化
}

/******************************************************************************/
/*                               MODE配置																			*/
/*											输入参数为0-->High-Speed mode													*/
/*										输入参数为1-->High-Resolution mode											*/
/*											输入参数为'z'-->Low-Power mode												*/
/******************************************************************************/

void ads1271_mode(char mode)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟

	if(0 == mode)//输出0
  {
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//GPIOB11
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
		GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO
		GPIO_ResetBits(GPIOB,GPIO_Pin_11);//置0
	}
	else if(1 == mode)//输出1
  {
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//GPIOB11
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
		GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO
		GPIO_SetBits(GPIOB,GPIO_Pin_11);//置1
	}
	else if('z' == mode)//悬浮
	{
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//GPIOB11
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输出模式
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//悬浮
		GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO
	}
	
}

/******************************************************************************/
/*                               SYNC配置                                     */
/******************************************************************************/

void sync_config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;//GPIOB15
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO
}

/******************************************************************************/
/*                             低功耗 自校准                                  */
/******************************************************************************/

void pdwn_calibration(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_15);
	delay_ms(20);
	GPIO_ResetBits(GPIOB,GPIO_Pin_15);
	delay_ms(20);
	GPIO_SetBits(GPIOB,GPIO_Pin_15);
}

/******************************************************************************/
/*                                 同步                                       */
/******************************************************************************/

void sync(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_15);
	delay_ms(20);
	GPIO_ResetBits(GPIOB,GPIO_Pin_15);
	delay_us(1);
	GPIO_SetBits(GPIOB,GPIO_Pin_15);
}
