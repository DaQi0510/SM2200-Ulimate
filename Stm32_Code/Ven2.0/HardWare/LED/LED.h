#ifndef __LED_H
#define __LED_H

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "sys.h"
#include "delay.h"

#define LED1 PEout(1)	// 红色
#define LED2 PBout(9)	//蓝色
#define LED3 PBout(8)	// 红色
#define LED4 PEout(0)	//蓝色
#define LED5 PBout(7)	//蓝色

#define NetPower_On PGout(7)=1;  //网口电源开
#define NetPower_Off PGout(7)=0; //网口电源关

#define SM2200Power_On PGout(10)=0; //SM2200电源开
#define SM2200Power_Off PGout(10)=1; //SM2200电源关

void 	gpio_Init(void);
void Key_Init(void);


#endif



