#ifndef BUTTONS_H
#define BUTTONS_H

#include <stm32f4xx.h>

///Time in ms
/*
#define BOUNCE_TIME 40
#define HOLD_TIME 800
#define HOLD_REPEAT_TIME 250
#define CLICK_TIME 300
*/
#define BTN_MAX_COUNT 10
#define BOUNCE_TIME 2
#define HOLD_TIME 52
#define HOLD_REPEAT_TIME 17
#define CLICK_TIME 20

//Count and names of buttons
//#define BTN_COUNT 3


typedef struct buttons_phys_s
{
	uint16_t *pins;
	GPIO_TypeDef **ports;
	GPIOPuPd_TypeDef *pupd;
} buttons_phys_t;

int8_t ButtonInit(uint8_t count, buttons_phys_t *physical);
int8_t ButtonPeriphInit(uint8_t rcc_count, uint32_t *rcc);
uint8_t Buttons_IRQ(void);
uint8_t Button_onPress(uint8_t cur);
uint8_t Button_pressed(uint8_t cur);
uint8_t Button_onHold(uint8_t cur);
uint8_t Button_holded(uint8_t cur, uint8_t once);
uint8_t Button_released(uint8_t cur);
uint8_t Button_withRepeat(uint8_t cur);
uint8_t Button_clicks(uint8_t cur,uint8_t count);
uint8_t Button_getClicks(uint8_t cur,uint8_t consider_hold);
uint8_t Button_onPressCount(void);

#endif
