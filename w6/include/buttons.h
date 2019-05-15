#ifndef BUTTONS_H
#define BUTTONS_H

#include <stm32f4xx.h>

#define BTN_MAX_COUNT 10

/* Time in beacon interval */
#define BOUNCE_TIME 2
#define HOLD_TIME 52
#define HOLD_REPEAT_TIME 17
#define CLICK_TIME 20

/*! \brief  Contains params of physical buttons
 */
typedef struct buttons_phys_s
{
    uint16_t *pins;
    GPIO_TypeDef **ports;
    GPIOPuPd_TypeDef *pupd;
} buttons_phys_t;

/*! \brief  Inits button state machine
 *  \param count count of buttons
 *  \param physical ptr to array of buttons param
 */
int8_t buttons_init(uint8_t count, buttons_phys_t *physical);

/*! \brief  Inits physical buttons
 *  \param rcc_count count of RCC to init
 *  \param rcc ptr to array of RCC values
 */
int8_t buttons_periph_init(uint8_t rcc_count, uint32_t *rcc);

/*! \brief  Func which process all buttons
 */
uint8_t buttons_irq(void);

/*! \brief  Is the button pressed now
 */
uint8_t button_on_press(uint8_t cur);

/*! \brief  Was the button pressed last time
 */
uint8_t button_pressed(uint8_t cur);

/*! \brief  Is the button on hold
 */
uint8_t button_on_hold(uint8_t cur);

/*! \brief  Was button holded
 *  \params once don't clear this state after reading
 */
uint8_t button_holded(uint8_t cur, uint8_t once);

/*! \brief  Was the button released
 */
uint8_t button_released(uint8_t cur);

/*! \brief  Repeats button pressing over time, like PC keyboard
 */
uint8_t button_with_repeat(uint8_t cur);

/*! \brief  Returns true if count value matches the clicks
 */
uint8_t button_clicks(uint8_t cur, uint8_t count);

/*! \brief  Returns count of clicks
 *  \params consider_hold for using with Button_withRepeat function
 */
uint8_t button_get_clicks(uint8_t cur, uint8_t consider_hold);

/*! \brief  Returns current count of on press buttons
 */
 uint8_t button_on_press_count(void);

#endif /*BUTTONS_H*/
