#include "sys.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_it.h"

#include "delay.h"
#include "timer.h"
#include "led.h"
#include "ads1271.h"
#include "sram.h"
#include "ethernet.h"
#include "agreement.h"

int main(void)
{		
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(162);
	tim3_init(162);
	led_init();
	ads1271_init(0);
	sram_init();
	
	A0:
	ethernet_init();
	tcp_sever();
	while(!is_con());
	while(1)
	{
		
		if(deal_int() == 0)//INT处理部分
		{
			goto A0;//重启
		}
	
		if(deal_pre() == 0)//PRE处理部分
		{
			goto A0;//重启
		}

		if(deal_div() == 0)//DIV处理
		{
			goto A0;//重启
		}
	
		if(deal_sta() == 0)//STA处理
		{
			goto A0;//重启
		}
	
		while(1)
		{
			if(deal_data() == 0)//数据传输
			{
				goto A0;//重启
			}
		}
	}
}

