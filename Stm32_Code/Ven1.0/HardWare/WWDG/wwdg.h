#ifndef __WWDG_H
#define __WWDG_H
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_wwdg.h"
#include "sys.h"

void WWDG_Init(u8 tr,u8 wr,u32 fprer);
void WWDG_IRQHandler(void);
void Set_WWDG(void);

#endif
