#include "led.h" 

/******************************************************************************/
/*                              LED初始化                                     */
/******************************************************************************/

void led_init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOE时钟

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO
	
	GPIO_SetBits(GPIOE,GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5);//设置低，灯灭
}

/******************************************************************************/
/*                             红灯 1亮 0灭                                   */
/******************************************************************************/

void led_r(unsigned char status)
{
	if(status)
		GPIO_ResetBits(GPIOE,GPIO_Pin_3);
	else
		GPIO_SetBits(GPIOE,GPIO_Pin_3);
}

/******************************************************************************/
/*                             绿灯 1亮 0灭                                   */
/******************************************************************************/

void led_g(unsigned char status)
{
	if(status)
		GPIO_ResetBits(GPIOE,GPIO_Pin_4);
	else
		GPIO_SetBits(GPIOE,GPIO_Pin_4);
}

/******************************************************************************/
/*                             黄灯 1亮 0灭                                   */
/******************************************************************************/

void led_y(unsigned char status)
{
	if(status)
		GPIO_ResetBits(GPIOE,GPIO_Pin_5);
	else
		GPIO_SetBits(GPIOE,GPIO_Pin_5);
}

/******************************************************************************/
/*                               红灯 翻转                                    */
/******************************************************************************/

void led_r_toggle(void)
{
	GPIO_ToggleBits(GPIOE,GPIO_Pin_3);
}

/******************************************************************************/
/*                               绿灯 翻转                                    */
/******************************************************************************/

void led_g_toggle(void)
{
	GPIO_ToggleBits(GPIOE,GPIO_Pin_4);
}

/******************************************************************************/
/*                               黄灯 翻转                                    */
/******************************************************************************/

void led_y_toggle(void)
{
	GPIO_ToggleBits(GPIOE,GPIO_Pin_5);
}
