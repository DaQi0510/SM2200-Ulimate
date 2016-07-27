#ifndef __UART_H
#define __UART_H

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx.h"

void uart_init(u32 bound);
void USART_Send(u8 *Message,u8 Length);


#endif

