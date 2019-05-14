#include "buttons.h"
#include "string.h"

/*! \brief  States of state machine
 */
typedef enum machine_state_e
{
    M_WAITING,
    M_DEBOUNCING,
    M_CLICKS_COUNTING,
    M_HOLDING,
    M_WAIT_RELEASE
} machine_state_t; 

/*! \brief  Contains all about button
 */
typedef struct button_state_s 
{
    machine_state_t state;
    uint8_t on_press;
    uint8_t pressed;
    uint8_t on_hold;
    uint8_t holded;
    uint8_t released;
    uint8_t hold_repeat;
    uint8_t clicks;
    uint32_t click_timer;
    uint32_t timer;
} button_state_t;

static button_state_t btns[BTN_MAX_COUNT];
buttons_phys_t *btns_phys;
static uint8_t btn_count;

uint8_t button_fsm(uint8_t cur);

static uint8_t m_wait_check(button_state_t *button, uint8_t phys_state);
static uint8_t m_debounce_check(button_state_t *button, uint8_t phys_state);
static uint8_t m_clicks_check(button_state_t *button, uint8_t phys_state);
static uint8_t m_hold_check(button_state_t *button, uint8_t phys_state);
static uint8_t m_release_check(button_state_t *button, uint8_t phys_state);

int8_t buttons_periph_init(uint8_t rcc_count, uint32_t *rcc)
{
    uint8_t cur;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

    if (btn_count == 0)
    {
        return -1;
    }
    
    /* Enable RSS's */
    for (cur = 0; cur < rcc_count; cur++)
    {
        RCC_AHB1PeriphClockCmd((uint32_t)rcc[cur], ENABLE);
    }
    /* Init physical buttons */
    for (cur = 0; cur < btn_count; cur++)
    {
        GPIO_InitStructure.GPIO_PuPd = btns_phys->pupd[cur];
        GPIO_InitStructure.GPIO_Pin = btns_phys->pins[cur];
        GPIO_Init(btns_phys->ports[cur], &GPIO_InitStructure);
    }
    return 0;
}

int8_t buttons_init(uint8_t count, buttons_phys_t *physical)
{
    if (count == 0)
    {
        return -1;
    }
    else
    {
        btn_count = (count > BTN_MAX_COUNT) ? BTN_MAX_COUNT : count;
        memset(&btns, 0, sizeof(btns));
        btns_phys = physical;
    }
    return 0;
}

/*! \brief  Button state machine which process each button
 */
uint8_t button_fsm(uint8_t cur)
{
    uint8_t btn_update = 0;
    /* read current btn state */
    uint8_t phys_state = GPIO_ReadInputDataBit(
        btns_phys->ports[cur], btns_phys->pins[cur]);
    /* and inverse its state if needed */
    if (btns_phys->pupd[cur] == GPIO_PuPd_UP)
    {
        phys_state ^= 1;
    }
    switch (btns[cur].state)
    {
    case M_WAITING:
        btn_update = m_wait_check(btns + cur, phys_state);
        break;
    case M_DEBOUNCING:
        btn_update = m_debounce_check(btns + cur, phys_state);
        break;
    case M_CLICKS_COUNTING:
        btn_update = m_clicks_check(btns + cur, phys_state);
        break;
    case M_HOLDING:
        btn_update = m_hold_check(btns + cur, phys_state);
       break;
    case M_WAIT_RELEASE:
        btn_update = m_release_check(btns + cur, phys_state);
        break;
    default:
        break;
    }
    return btn_update;
}

static uint8_t m_wait_check(button_state_t *button, uint8_t phys_state)
{
    if (phys_state)
    {
        button->timer = 0;
        button->pressed = 0;
        button->state = M_DEBOUNCING;
    }
    if (button->click_timer != 0)
    {
        if (button->click_timer >= CLICK_TIME)
        {
            button->click_timer = 0;
            return 1;
        }
        else
        {
            button->click_timer++;
        }
    }
    return 0;
}

static uint8_t m_debounce_check(button_state_t *button, uint8_t phys_state)
{
    button->timer++;
    if (button->timer >= BOUNCE_TIME)
    {
        if (phys_state)
        {
            if (button->click_timer == 0)
            {
                button->clicks = 0;
            }
            button->on_press = 1;
            button->pressed = 1;
            button->released = 0;
            button->holded = 0;
            button->hold_repeat = 1;
            button->click_timer = BOUNCE_TIME;
            button->state = M_CLICKS_COUNTING;
            return 1;
        }
        else
        {
            button->state = M_WAITING;
        }
    }
    return 0;
}

static uint8_t m_clicks_check(button_state_t *button, uint8_t phys_state)
{
    button->click_timer++;
    if (phys_state)
    {
        button->timer++;
        if (button->click_timer >= CLICK_TIME)
        {
            button->clicks += 1;
            button->click_timer = 0;
            button->state = M_HOLDING;
            return 1;
        }
    }
    else
    {
        button->on_press = 0;
        button->released = 1;
        button->clicks++;
        button->state = M_WAITING;
        return 1;
    }
    return 0;
}

static uint8_t m_hold_check(button_state_t *button, uint8_t phys_state)
{
    if (phys_state)
    {
        button->timer++;
        if (button->timer >= HOLD_TIME)
        {
            button->on_hold = 1;
            button->holded = 1;
            button->hold_repeat = 1;
            button->timer = 0;
            button->state = M_WAIT_RELEASE;
            return 1;
        }
    }
    else
    {
        button->on_press = 0;
        button->released = 1;
        button->state = M_WAITING;
        return 1;
    }
    return 0;
}

static uint8_t m_release_check(button_state_t *button, uint8_t phys_state)
{
    if (phys_state)
    {
        button->timer++;
        if (button->timer >= HOLD_REPEAT_TIME)
        {
            button->hold_repeat = 1;
            button->timer = 0;
            return 1;
        }
    }
    else
    {
        button->on_press = 0;
        button->on_hold = 0;
        button->released = 1;
        button->hold_repeat = 0;
        button->state = M_WAITING;
        return 1;
    }
    return 0;
}

/* Public functions */

/*! \brief  Func which process all buttons
 */
uint8_t buttons_irq(void)
{
    uint8_t cur;
    uint8_t flag = 0;
    for (cur = 0; cur < btn_count; cur++)
    {
        if (button_fsm(cur))
        {
            flag++;
        }
    }
    return flag;
}

/*! \brief  Is the button pressed now
 */
uint8_t button_on_press(uint8_t cur)
{
    if (btns[cur].on_press == 1)
    {
        return 1;
    }
    return 0;
}

/*! \brief  Was the button pressed last time
 */
uint8_t button_pressed(uint8_t cur)
{
    if (btns[cur].pressed == 1)
    {
        btns[cur].pressed = 0;
        return 1;
    }
    return 0;
}

/*! \brief  Is the button on hold
 */
uint8_t button_on_hold(uint8_t cur)
{
    if (btns[cur].on_hold == 1)
    {
        return 1;
    }
    return 0;
}

/*! \brief  Was the button released
 */
uint8_t button_released(uint8_t cur)
{
    if (btns[cur].released == 1)
    {
        btns[cur].released = 0;
        return 1;
    }
    return 0;
}

/*! \brief  Repeats button pressing over time, like PC keyboard
 */
uint8_t button_with_repeat(uint8_t cur)
{
    if (btns[cur].hold_repeat == 1)
    {
        btns[cur].hold_repeat = 0;
        return 1;
    }
    return 0;
}

/*! \brief  Returns true if count value matches the clicks
 */
uint8_t button_clicks(uint8_t cur, uint8_t count)
{
    if (btns[cur].clicks == count)
    {
        btns[cur].click_timer = 0;
        btns[cur].clicks = 0;
        return 1;
    }
    return 0;
}

/*! \brief  Was button holded
 *  \params once don't clear this state after reading
 */
uint8_t button_holded(uint8_t cur, uint8_t once)
{
    if (btns[cur].holded)
    {
        if (once)
        {
            btns[cur].holded = 0;
        }
        return 1;
    }
    return 0;
}

/*! \brief  Returns count of clicks
 *  \params consider_hold for using with Button_withRepeat function
 */
uint8_t button_get_clicks(uint8_t cur, uint8_t consider_hold)
{
    uint8_t tmp;
    if (btns[cur].clicks > 0 && btns[cur].click_timer == 0)
    {
        if (consider_hold)
        {
            if (btns[cur].state == M_HOLDING)
            {
                return 0;
            }
            else if (btns[cur].state == M_WAIT_RELEASE)
            {
                return btns[cur].clicks;
            }
            else if (btns[cur].holded == 0 && btns[cur].state == M_WAITING)
            {
                tmp = btns[cur].clicks;
                btns[cur].clicks = 0;
                return tmp;
            }
        }
        else
        {
            tmp = btns[cur].clicks;
            btns[cur].clicks = 0;
            return tmp;
        }
    }
    return 0;
}

/*! \brief  Returns current count of on press buttons
 */
uint8_t button_on_press_count(void)
{
    uint8_t cur, count;
    for(cur = 0; cur < btn_count; cur++)
    {
        count += btns[cur].on_press;
    } 
    return count;
}
