#ifndef LED_H
#define LED_H

#include <stm32f4xx.h>

///\brief Sets what timer will be used
///\details Determines, which timer OC1-3 will be set brightness value
#define USING_TIMER TIM1

///\brief Sets period of timer
#define TIM_PERIOD 0xFF

/// Sets levels of brightness count
#define INTENSITY_STEP_COUNT 5

/**
        \brief Performs base init

        Variables resets, LED not lights
*/
void LED_init_val(void);

/**
        \brief Inits LED of the existing prototype

        \warning Use this func only if LED is connected to PA8-10 of TIM1 (OC
   outputs). If not, init it manually.

        Inits GPIO PA8-10 as AF TIM1, inits TIM1 OC1-3 for PWM output
*/
void LED_init_periph(void);

/**
        \brief Sets total brightness

        May be used for dimming when the color temperature is set
*/
void LED_set_intensity(uint16_t intensity);

/**
        \brief Sets color by RGB value
        \param *color array of Red, Green and Blue colors in range of 0 to 255
*/
void LED_set_color_RGB(uint8_t *rgb);

/**
        \brief Sets color by HEX color code
        \param value where 0xRRGGBB consist of values of Red, Green and Blue
   respectively
*/
void LED_set_color_HEX(uint32_t hex);

#endif