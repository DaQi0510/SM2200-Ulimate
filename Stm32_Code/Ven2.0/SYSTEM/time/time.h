#ifndef _TIME_H
#define _TIME_H

#include "stm32f4xx_tim.h"
#include "sys.h"
#include "stm32f4xx_rcc.h"

void TIM3_Init(u16 arr,u16 psc) ;
void TIM4_Init(void);
void TIM5_Init(void);

#endif
