#include "led.h"
#include "globals.h"

#include <stm32f4xx_tim.h>

#define MAX_INTENSITY TIM_PERIOD

static void LED_set_color_raw(void);
static uint16_t LED_calc_color(uint16_t *value);

static uint16_t intens_curr_step;
static uint16_t rgb_curr[3];

/**
        \brief Performs base init

        Variables resets, LED not lights
*/
void LED_init_val(void)
{
    rgb_curr[0] = 0;
    rgb_curr[1] = 0;
    rgb_curr[2] = 0;
    LED_set_intensity(INTENSITY_STEP_COUNT);
}

/**
        \brief Inits LED of the existing prototype

        \warning Use this func only if LED is connected to PA8-10 of TIM1 (OC
   outputs). If not, init it manually.

        Inits GPIO PA8-10 as AF TIM1, inits TIM1 OC1-3 for PWM output
*/
void LED_init_periph(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

    // init board LEDs for PWM tim1
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_TIM1);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // init TIM1
    TIM_TimeBaseInitTypeDef TIM_BaseStruct;
    TIM_TimeBaseStructInit(&TIM_BaseStruct);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    TIM_BaseStruct.TIM_Prescaler = 0x1500; // optimal for green
    TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_BaseStruct.TIM_Period = TIM_PERIOD;
    TIM_BaseStruct.TIM_ClockDivision = 0;
    TIM_BaseStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1,
                     &TIM_BaseStruct); // Initialize timer with chosen settings

    // OC init PWM
    TIM_OCInitTypeDef TIM_OCStruct;
    TIM_OCStructInit(&TIM_OCStruct);
    TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OCStruct.TIM_Pulse = 0;
    TIM_OC1Init(TIM1, &TIM_OCStruct);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC2Init(TIM1, &TIM_OCStruct);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC3Init(TIM1, &TIM_OCStruct);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE); // Start timer

    LED_init_val();
}

/**
        \brief Preparing color value to output

        Change raw value of brightness to eye-likely and sets level of
   brightness depends of current intensity value

        \param raw color value
*/
static uint16_t LED_calc_color(uint16_t *value)
{
    if (intens_curr_step == 0)
    { // intens set to 0
        return 0;
    }
    else if (intens_curr_step >= INTENSITY_STEP_COUNT)
    {
        if (*value >= MAX_INTENSITY)
        { // intens_curr_step is max & brightness is max
            return MAX_INTENSITY;
        }
        else
        { // intens_curr_step is max
            return ((uint32_t)(*value) * (*value) / MAX_INTENSITY);
        }
    }
    else
    { // other case
        return ((((uint32_t)(*value) * (*value) / INTENSITY_STEP_COUNT) *
                 (intens_curr_step)) /
                MAX_INTENSITY);
    }
}

/**
        \brief Sets raw value of color

        Sets pulse width of PWM for 3 channels in eye-likely form

        \param *rgb array of Red, Green and Blue in raw form
*/
static void LED_set_color_raw(void)
{
    TIM_SetCompare1(USING_TIMER, LED_calc_color(rgb_curr));
    TIM_SetCompare2(USING_TIMER, LED_calc_color(rgb_curr + 1));
    TIM_SetCompare3(USING_TIMER, LED_calc_color(rgb_curr + 2));
}

/**
        \brief Sets total brightness

        May be used for dimming when the color temperature is set
*/
void LED_set_intensity(uint16_t intensity)
{
    intens_curr_step = intensity;
    LED_set_color_raw();
}

/**
        \brief Sets color by RGB value
        \param *color array of Red, Green and Blue colors in range of 0 to 255
*/
void LED_set_color_RGB(uint8_t *color)
{
    uint8_t i;
    for (i = 0; i < 3; i++)
    {
        if (color[i] == 0)
            rgb_curr[i] = 0;
        else if (color[i] == 0xFF)
            rgb_curr[i] = MAX_INTENSITY;
        else
            rgb_curr[i] = (MAX_INTENSITY * color[i]) / 0xFF;
    }
    LED_set_color_raw();
}

/**
        \brief Sets color by HEX color code
        \param value where 0xRRGGBB consist of values of Red, Green and Blue
   respectively
*/
void LED_set_color_HEX(uint32_t color)
{
    uint8_t rgb[3];
    rgb[0] = color >> 16;
    rgb[1] = (color >> 8) & 0xFF;
    rgb[2] = color & 0xFF;
    LED_set_color_RGB(rgb);
}
