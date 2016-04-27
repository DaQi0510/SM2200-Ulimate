#ifndef __LED_H
#define __LED_H

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "sys.h"
#include "delay.h"

#define LED1 PEout(1)	// ��ɫ
#define LED2 PEout(0)	//��ɫ
#define LED3 PBout(9)	// ��ɫ
#define LED4 PBout(8)	//��ɫ
#define LED5 PBout(7)	//��ɫ

#define NetPower_On PGout(7)=1; //SM2200��Դ��
#define NetPower_Off PGout(7)=0; //SM2200��Դ��

#define SM2200Power_On PAout(11)=0; //SM2200��Դ��
#define SM2200Power_Off PAout(11)=1; //SM2200��Դ��

void 	gpio_Init(void);
void Key_Init(void);


#endif



