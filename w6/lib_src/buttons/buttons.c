#include "buttons.h"
//#include "stdlib.h"
#include "string.h"

//#define LED_FIRST_PIN GPIO_Pin_12 //for debug malloc
//#define LED_PORT GPIOD

typedef enum {bWaiting, bDebouncing, bClicksCounting, bHolding, bWaitRelease} ButtonsState; ///States of state machine

typedef struct button_state ///Contains all about button
{
	ButtonsState bState;
	uint8_t bOnPress;
	uint8_t bPressed;
	uint8_t bOnHold;
	uint8_t bHolded;
	uint8_t bReleased;
	uint8_t bHoldRepeat;
	uint8_t bClicks;
	uint32_t bClickTimer;
	uint32_t bTimer;
} ButtonState;

//Contains declared buttons
static ButtonState btns[BTN_MAX_COUNT];
buttons_phys_t *btns_phys;
static uint8_t btn_count;
static uint8_t btn_pressed;

uint8_t Button_FSM(uint8_t cur);

int8_t ButtonPeriphInit(uint8_t rcc_count, uint32_t *rcc)
{
	uint8_t cur;
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	
	if(btn_count == 0) {
		return -1;
	}
	else {
		//Enable RSS's
		for(cur = 0; cur < rcc_count; cur++) {
			RCC_AHB1PeriphClockCmd((uint32_t)rcc[cur], ENABLE);
		}
		//Init buttons
		for(cur = 0; cur < btn_count; cur++) {
			GPIO_InitStructure.GPIO_PuPd = btns_phys->pupd[cur];
			GPIO_InitStructure.GPIO_Pin = btns_phys->pins[cur];
			GPIO_Init(btns_phys->ports[cur], &GPIO_InitStructure);
		}
	}
	return 0;
}


int8_t ButtonInit(uint8_t count, buttons_phys_t *physical)
{
	//uint8_t cur;
	
	if(count == 0) {
		return -1;
	}
	else {
		btn_count = (count > BTN_MAX_COUNT) ? BTN_MAX_COUNT : count;
		memset(&btns, 0, sizeof(btns));
		btn_pressed = 0;
		btns_phys = physical;
	}
	return 0;
}

///Button state machine which process each button
uint8_t Button_FSM(uint8_t cur) {
	uint8_t btn_update = 0;
	uint8_t btn_state = GPIO_ReadInputDataBit(btns_phys->ports[cur], btns_phys->pins[cur]); //read current btn state
	if(btns_phys->pupd[cur] == GPIO_PuPd_UP) btn_state ^= 1;
	switch(btns[cur].bState) {
	case bWaiting:
		if(btn_state) {
			btns[cur].bTimer = 0;
			btns[cur].bPressed = 0;
			btns[cur].bState = bDebouncing;
		}
		if(btns[cur].bClickTimer != 0) {
			if(btns[cur].bClickTimer >= CLICK_TIME) {
				btns[cur].bClickTimer = 0;
				btn_update = 1;
			}
			else {
				btns[cur].bClickTimer++;
			}
		}
		break;
	case bDebouncing:
		btns[cur].bTimer++;
		if(btns[cur].bTimer >= BOUNCE_TIME) {
			if(btn_state) {
				if(btns[cur].bClickTimer == 0) {
					btns[cur].bClicks = 0;
				}
				btns[cur].bOnPress = 1;
				btns[cur].bPressed = 1;
				btns[cur].bReleased = 0;
				btns[cur].bHolded = 0;
				btns[cur].bHoldRepeat = 1;
				btns[cur].bClickTimer = BOUNCE_TIME;
				btns[cur].bState = bClicksCounting;
				btn_update = 1;
				btn_pressed++;
			}
			else {
				btns[cur].bState = bWaiting;
			}
		}
		break;
	case bClicksCounting:
		btns[cur].bClickTimer++;
		if(btn_state) {
			btns[cur].bTimer++;
			if(btns[cur].bClickTimer >= CLICK_TIME){
				btns[cur].bClicks += 1;
				btns[cur].bClickTimer = 0;
				btns[cur].bState = bHolding;
				btn_update = 1;
			}
		}
		else {
			btns[cur].bOnPress = 0;
			btns[cur].bReleased = 1;
			btns[cur].bClicks++;
			btns[cur].bState = bWaiting;
			btn_update = 1;
			btn_pressed--;
		}
		break;
	case bHolding:
		if(btn_state) {
			btns[cur].bTimer++;
			if(btns[cur].bTimer >= HOLD_TIME){
				btns[cur].bOnHold = 1;
				btns[cur].bHolded = 1;
				btns[cur].bHoldRepeat = 1;
				btns[cur].bTimer = 0;
				btns[cur].bState = bWaitRelease;
				btn_update = 1;
			}
		}
		else {
			btns[cur].bOnPress = 0;
			btns[cur].bReleased = 1;
			btns[cur].bState = bWaiting;
			btn_update = 1;
			btn_pressed--;
		}
		break;
	case bWaitRelease:
		if(btn_state) {
			btns[cur].bTimer++;
			if(btns[cur].bTimer >= HOLD_REPEAT_TIME) {
				btns[cur].bHoldRepeat = 1;
				btns[cur].bTimer = 0;
				btn_update = 1;
			}
		}
		else {
			btns[cur].bOnPress = 0;
			btns[cur].bOnHold = 0;
			btns[cur].bReleased = 1;
			btns[cur].bHoldRepeat = 0;
			btns[cur].bState = bWaiting;
			btn_update = 1;
			btn_pressed--;
		}
		break;
	default: break;
	}
	return btn_update;
}

///Func which process all buttons
uint8_t Buttons_IRQ(void) {
	uint8_t cur;
	uint8_t flag = 0;
	for(cur = 0; cur < btn_count; cur++) {
		if(Button_FSM(cur))
		{
			flag++;
		}
	}
	return flag;
}

///Is the button pressed now
uint8_t Button_onPress(uint8_t cur) {
	if(btns[cur].bOnPress == 1) {
		return 1;
	}
	return 0;
}

///Was the button pressed last time
uint8_t Button_pressed(uint8_t cur) {
	if(btns[cur].bPressed == 1) {
		btns[cur].bPressed = 0;
		return 1;
	}
	return 0;
}

///Is the button on hold
uint8_t Button_onHold(uint8_t cur) {
	if(btns[cur].bOnHold == 1) {
		return 1;
	}
	return 0;
}

///Was the button released
uint8_t Button_released(uint8_t cur) {
	if(btns[cur].bReleased == 1) {
		btns[cur].bReleased = 0;
		return 1;
	}
	return 0;
}

///Repeats button pressing over time, like PC keyboard
uint8_t Button_withRepeat(uint8_t cur) {
	if(btns[cur].bHoldRepeat == 1) {
		btns[cur].bHoldRepeat = 0;
		return 1;
	}
	return 0;
}

///Returns true if count value matches the clicks
uint8_t Button_clicks(uint8_t cur,uint8_t count) {
	if(btns[cur].bClicks == count) {
		btns[cur].bClickTimer = 0;
		btns[cur].bClicks = 0;
		return 1;
	}
	return 0;
}

///Was button holded
///\params once don't clear this state after reading
uint8_t Button_holded(uint8_t cur, uint8_t once) {
	if(btns[cur].bHolded) {
		if(once) {
			btns[cur].bHolded = 0;
		}
		return 1;
	}
	return 0;
}

///Returns count of clicks
///\params consider_hold for using with Button_withRepeat function
uint8_t Button_getClicks(uint8_t cur,uint8_t consider_hold) {
	uint8_t tmp;
	if(btns[cur].bClicks > 0 && btns[cur].bClickTimer == 0) {
		if(consider_hold) {
			if(btns[cur].bState == bHolding) {
				return 0;
			}
			else if(btns[cur].bState == bWaitRelease) {
				return btns[cur].bClicks;
			}
			else if(btns[cur].bHolded == 0 && btns[cur].bState == bWaiting) {
				tmp = btns[cur].bClicks;
				btns[cur].bClicks = 0;
				return tmp;
			}
		}
		else {
			tmp = btns[cur].bClicks;
			btns[cur].bClicks = 0;
			return tmp;
		}
	}
	return 0;
}

uint8_t Button_onPressCount(void) {
	return btn_pressed;
}
