#ifndef LEDS_H_
#define LEDS_H_

#define LED_COUNT 4
#define LED_FADE_SPEED 15

#include "main.h"
#include "stm32f4xx.h"

void led_set(uint8_t channel, uint16_t value);
void led_fade_out(uint8_t channel, uint16_t step);
void led_set_max(uint8_t channel);

#endif /* LEDS_H_ */
