#ifndef BUTTONS_H
#define BUTTONS_H

#include <stm32f4xx.h>

/* Time in beacon interval */
#define BTN_MAX_COUNT 10
#define BOUNCE_TIME 2
#define HOLD_TIME 52
#define HOLD_REPEAT_TIME 17
#define CLICK_TIME 20

typedef struct buttons_phys_s
{
    uint16_t *pins;
    GPIO_TypeDef **ports;
    GPIOPuPd_TypeDef *pupd;
} buttons_phys_t;

int8_t buttons_init(uint8_t count, buttons_phys_t *physical);
int8_t buttons_periph_init(uint8_t rcc_count, uint32_t *rcc);
uint8_t buttons_irq(void);
uint8_t button_on_press(uint8_t cur);
uint8_t button_pressed(uint8_t cur);
uint8_t button_on_hold(uint8_t cur);
uint8_t button_holded(uint8_t cur, uint8_t once);
uint8_t button_released(uint8_t cur);
uint8_t button_with_repeat(uint8_t cur);
uint8_t button_clicks(uint8_t cur, uint8_t count);
uint8_t button_get_clicks(uint8_t cur, uint8_t consider_hold);
uint8_t button_on_press_count(void);

#endif /*BUTTONS_H*/
