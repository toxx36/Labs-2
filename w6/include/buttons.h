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

#define BOUNCE_TIME 2
#define HOLD_TIME 52
#define HOLD_REPEAT_TIME 17
#define CLICK_TIME 20

//Count and names of buttons
//#define BTN_COUNT 3
typedef enum {bBoard,bLeft,bRight} Buttons;

typedef struct buttons_init_s
{
	uint16_t *pins;
	GPIO_TypeDef **ports;
	GPIOPuPd_TypeDef *pupd;
} buttons_init_t;

int8_t ButtonInit(uint8_t count, buttons_init_t *init_btn, uint8_t rcc_count, uint32_t *rcc);
uint8_t Buttons_IRQ(void);
uint8_t Button_onPress(Buttons cur);
uint8_t Button_pressed(Buttons cur);
uint8_t Button_onHold(Buttons cur);
uint8_t Button_holded(Buttons cur, uint8_t once);
uint8_t Button_released(Buttons cur);
uint8_t Button_withRepeat(Buttons cur);
uint8_t Button_clicks(Buttons cur,uint8_t count);
uint8_t Button_getClicks(Buttons cur,uint8_t consider_hold);
uint8_t Button_onPressCount(void);

#endif
