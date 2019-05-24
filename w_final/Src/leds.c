#include "leds.h"

static uint16_t led_brightness[LED_COUNT] = {0};

void led_set(uint8_t channel, uint16_t value)
{
	uint16_t brightness;
	if(value < MAX_INTENSITY)
	{
		brightness = ((uint32_t)(value) * (value) / MAX_INTENSITY);
	}
	else
	{
		brightness = MAX_INTENSITY;
	}
	switch(channel)
	{
	case 1:
		LL_TIM_OC_SetCompareCH3(TIM4, brightness);
		break;
	case 2:
		LL_TIM_OC_SetCompareCH2(TIM4, brightness);
		break;
	case 3:
		LL_TIM_OC_SetCompareCH1(TIM4, brightness);
		break;
	case 4:
		LL_TIM_OC_SetCompareCH4(TIM4, brightness);
		break;
	default:
		LL_TIM_OC_SetCompareCH1(TIM4, brightness);
		LL_TIM_OC_SetCompareCH2(TIM4, brightness);
		LL_TIM_OC_SetCompareCH3(TIM4, brightness);
		LL_TIM_OC_SetCompareCH4(TIM4, brightness);
	}
}

void led_fade_out(uint8_t channel, uint16_t step)
{
	if(channel > 0 && channel <= LED_COUNT)
	{
		if(led_brightness[channel - 1] >= step)
		{
			led_brightness[channel - 1] -= step;
		}
		else
		{
			led_brightness[channel - 1] = 0;
		}
		led_set(channel, led_brightness[channel - 1]);
	}
}

void led_set_max(uint8_t channel)
{
	size_t i;
	if(channel > 0 && channel <= LED_COUNT)
	{
		led_brightness[channel - 1] = MAX_INTENSITY;
		led_set(channel, led_brightness[channel - 1]);
	}
	else if(channel == 0)
	{
		for(i = 0; i < LED_COUNT; i++)
		{
			led_brightness[i] = MAX_INTENSITY;
		}
		led_set(0, MAX_INTENSITY);
	}
}

