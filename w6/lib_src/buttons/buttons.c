#include "buttons.h"
#include "string.h"

typedef enum machine_state_e
{
    M_WAITING,
    M_DEBOUNCING,
    M_CLICKS_COUNTING,
    M_HOLDING,
    M_WAIT_RELEASE
} machine_state_t; /// States of state machine

typedef struct button_state_s /// Contains all about button
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

// Contains declared buttons
static button_state_t btns[BTN_MAX_COUNT];
buttons_phys_t *btns_phys;
static uint8_t btn_count;
static uint8_t btn_pressed;

uint8_t button_fsm(uint8_t cur);

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
    else
    {
        // Enable RSS's
        for (cur = 0; cur < rcc_count; cur++)
        {
            RCC_AHB1PeriphClockCmd((uint32_t)rcc[cur], ENABLE);
        }
        // Init buttons
        for (cur = 0; cur < btn_count; cur++)
        {
            GPIO_InitStructure.GPIO_PuPd = btns_phys->pupd[cur];
            GPIO_InitStructure.GPIO_Pin = btns_phys->pins[cur];
            GPIO_Init(btns_phys->ports[cur], &GPIO_InitStructure);
        }
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
        btn_pressed = 0;
        btns_phys = physical;
    }
    return 0;
}

/// Button state machine which process each button
uint8_t button_fsm(uint8_t cur)
{
    uint8_t btn_update = 0;
    uint8_t btn_state = GPIO_ReadInputDataBit(
        btns_phys->ports[cur], btns_phys->pins[cur]); // read current btn state
    if (btns_phys->pupd[cur] == GPIO_PuPd_UP)
        btn_state ^= 1;
    switch (btns[cur].state)
    {
    case M_WAITING:
        if (btn_state)
        {
            btns[cur].timer = 0;
            btns[cur].pressed = 0;
            btns[cur].state = M_DEBOUNCING;
        }
        if (btns[cur].click_timer != 0)
        {
            if (btns[cur].click_timer >= CLICK_TIME)
            {
                btns[cur].click_timer = 0;
                btn_update = 1;
            }
            else
            {
                btns[cur].click_timer++;
            }
        }
        break;
    case M_DEBOUNCING:
        btns[cur].timer++;
        if (btns[cur].timer >= BOUNCE_TIME)
        {
            if (btn_state)
            {
                if (btns[cur].click_timer == 0)
                {
                    btns[cur].clicks = 0;
                }
                btns[cur].on_press = 1;
                btns[cur].pressed = 1;
                btns[cur].released = 0;
                btns[cur].holded = 0;
                btns[cur].hold_repeat = 1;
                btns[cur].click_timer = BOUNCE_TIME;
                btns[cur].state = M_CLICKS_COUNTING;
                btn_update = 1;
                btn_pressed++;
            }
            else
            {
                btns[cur].state = M_WAITING;
            }
        }
        break;
    case M_CLICKS_COUNTING:
        btns[cur].click_timer++;
        if (btn_state)
        {
            btns[cur].timer++;
            if (btns[cur].click_timer >= CLICK_TIME)
            {
                btns[cur].clicks += 1;
                btns[cur].click_timer = 0;
                btns[cur].state = M_HOLDING;
                btn_update = 1;
            }
        }
        else
        {
            btns[cur].on_press = 0;
            btns[cur].released = 1;
            btns[cur].clicks++;
            btns[cur].state = M_WAITING;
            btn_update = 1;
            btn_pressed--;
        }
        break;
    case M_HOLDING:
        if (btn_state)
        {
            btns[cur].timer++;
            if (btns[cur].timer >= HOLD_TIME)
            {
                btns[cur].on_hold = 1;
                btns[cur].holded = 1;
                btns[cur].hold_repeat = 1;
                btns[cur].timer = 0;
                btns[cur].state = M_WAIT_RELEASE;
                btn_update = 1;
            }
        }
        else
        {
            btns[cur].on_press = 0;
            btns[cur].released = 1;
            btns[cur].state = M_WAITING;
            btn_update = 1;
            btn_pressed--;
        }
        break;
    case M_WAIT_RELEASE:
        if (btn_state)
        {
            btns[cur].timer++;
            if (btns[cur].timer >= HOLD_REPEAT_TIME)
            {
                btns[cur].hold_repeat = 1;
                btns[cur].timer = 0;
                btn_update = 1;
            }
        }
        else
        {
            btns[cur].on_press = 0;
            btns[cur].on_hold = 0;
            btns[cur].released = 1;
            btns[cur].hold_repeat = 0;
            btns[cur].state = M_WAITING;
            btn_update = 1;
            btn_pressed--;
        }
        break;
    default:
        break;
    }
    return btn_update;
}

/// Func which process all buttons
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

/// Is the button pressed now
uint8_t button_on_press(uint8_t cur)
{
    if (btns[cur].on_press == 1)
    {
        return 1;
    }
    return 0;
}

/// Was the button pressed last time
uint8_t button_pressed(uint8_t cur)
{
    if (btns[cur].pressed == 1)
    {
        btns[cur].pressed = 0;
        return 1;
    }
    return 0;
}

/// Is the button on hold
uint8_t button_on_hold(uint8_t cur)
{
    if (btns[cur].on_hold == 1)
    {
        return 1;
    }
    return 0;
}

/// Was the button released
uint8_t button_released(uint8_t cur)
{
    if (btns[cur].released == 1)
    {
        btns[cur].released = 0;
        return 1;
    }
    return 0;
}

/// Repeats button pressing over time, like PC keyboard
uint8_t button_with_repeat(uint8_t cur)
{
    if (btns[cur].hold_repeat == 1)
    {
        btns[cur].hold_repeat = 0;
        return 1;
    }
    return 0;
}

/// Returns true if count value matches the clicks
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

/// Was button holded
///\params once don't clear this state after reading
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

/// Returns count of clicks
///\params consider_hold for using with Button_withRepeat function
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

uint8_t button_on_press_count(void)
{
    return btn_pressed;
}
