#ifndef __LED_H
#define __LED_H
#include "sys.h"

void led_init(void);
void led_r(unsigned char status);
void led_g(unsigned char status);
void led_y(unsigned char status);
void led_r_toggle(void);
void led_g_toggle(void);
void led_y_toggle(void);

#endif
