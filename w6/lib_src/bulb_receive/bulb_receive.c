#include "bulb_receive.h"

static void bulb_turn_on(void);
static void bulb_turn_off(void);
static void bulb_toggle(void);
static void bulb_toggle_color(void);
static void bulb_brightness_up(void);
static void bulb_brightness_down(void);
static void bulb_brightness_level(bulb_brightness_t *bulb_brightness);

static zb_uint32_t colors[] = {0xFF0000, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0xFF00FF};
static zb_uint8_t bulb_state = BULB_OFF;
static zb_uint8_t bulb_cur_color = 0;
static zb_uint16_t bulb_intensity = 6;

#ifdef TEST_BTN
/* * * * buttons test * * * */
typedef enum {bBoard = 0, bLeft, bRight} Buttons;

void init_btn(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	/* Init LEDs on discovery board */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_ResetBits(LED_PORT,LED_FIRST_PIN<<0|LED_FIRST_PIN<<1|LED_FIRST_PIN<<2|LED_FIRST_PIN<<3);

	///The order matches the names of the buttons
	static uint16_t pins[] = {GPIO_Pin_0, GPIO_Pin_0, GPIO_Pin_1};
	static GPIO_TypeDef *ports[] = {GPIOA, GPIOE, GPIOE}; 
	static GPIOPuPd_TypeDef pupd[] = {GPIO_PuPd_DOWN, GPIO_PuPd_UP, GPIO_PuPd_UP};
	static buttons_phys_t buttons = {pins, ports, pupd};
	ButtonInit(3, &buttons);
	uint32_t rcc[] = {RCC_AHB1Periph_GPIOA, RCC_AHB1Periph_GPIOE};
	ButtonPeriphInit(2, rcc);
}

void buttons_click_test_cb(zb_uint8_t param) ZB_CALLBACK
{
	ZVUNUSED(param);
	static uint8_t prev_cnt = 0;
	if(Button_onPressCount() == 0)
	{
		prev_cnt = 0;
	}
	else if(Button_onPressCount() == 2)
	{
		prev_cnt = 2;
		if(Button_withRepeat(bRight)) 
		{
			bulb_toggle_color();
		}
	}
	if(prev_cnt != 2)
	{
		if(Button_onHold(bLeft) && Button_withRepeat(bLeft)) 
		{
			bulb_brightness_up();
		}
		else if (Button_onHold(bRight) && Button_pressed(bRight))
		{
			bulb_toggle();
		}
	}
}

void buttons_scan_cb(zb_uint8_t param) ZB_CALLBACK
{
	ZVUNUSED(param);
	if(Buttons_IRQ())
	{
		ZB_SCHEDULE_CALLBACK(buttons_click_test_cb, 0);
	}
	ZB_SCHEDULE_ALARM(buttons_scan_cb, 0, 1);
}
/* * * * * * * * */
#endif

void bulb_get_data(zb_uint8_t param) ZB_CALLBACK
{
	zb_uint8_t *ptr;
	zb_uint8_t len;
	bulb_cmd_t *bulb_cmd;
	bulb_brightness_t *bulb_brightness;
	
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);

	/* Remove APS header from the packet */
	ZB_APS_HDR_CUT_P(buf, ptr);
	len = ZB_BUF_LEN(buf);
	if(len == 0)
	{
		TRACE_MSG(TRACE_APS2, "### Buffer is empty!!!", (FMT__0));	
	}
	else if(len < sizeof(bulb_cmd_t))
	{
		TRACE_MSG(TRACE_APS2, "### Wrong command size in buffer!", (FMT__0));		
	}
	else
	{
		bulb_cmd = (bulb_cmd_t *)ZB_BUF_BEGIN(buf);
		switch(*bulb_cmd) 
		{
			case BULB_ON:
				TRACE_MSG(TRACE_APS2, "### Received command ON", (FMT__0));	
				bulb_turn_on();
				break;
			case BULB_OFF:
				TRACE_MSG(TRACE_APS2, "### Received command OFF", (FMT__0));	
				bulb_turn_off();
				break;
			case BULB_COLOR_TOGGLE:
				TRACE_MSG(TRACE_APS2, "### Received command TOGGLE COLOR", (FMT__0));	
				bulb_toggle_color();
				break;
			case BULB_TOGGLE:
				TRACE_MSG(TRACE_APS2, "### Received command TOGGLE", (FMT__0));	
				bulb_toggle();
				break;	
			case BULB_BRIGHTNESS_STEP_UP:
				TRACE_MSG(TRACE_APS2, "### Received command STEP UP", (FMT__0));
				bulb_brightness_up();					
				break;
			case BULB_BRIGHTNESS_STEP_DOWN:
				TRACE_MSG(TRACE_APS2, "### Received command STEP DOWN", (FMT__0));	
				bulb_brightness_down();
				break;
			case BULB_BRIGHTNESS_LEVEL:
				if(len != sizeof(bulb_brightness_t))
				{
					TRACE_MSG(TRACE_APS2, "### Error: received command SET LEVEL, but brightness level has wrong size", (FMT__0));	 
				}
				else
				{
					bulb_brightness = (bulb_brightness_t *)ZB_BUF_BEGIN(buf);
					TRACE_MSG(TRACE_APS2, "### Received command SET LEVEL, brightness = %d", (FMT__D, bulb_brightness->brightness));	 
					bulb_brightness_level(bulb_brightness);					
				}
				break;
			default:
				TRACE_MSG(TRACE_APS2, "### Received unknown command", (FMT__0));	
			break;
		}
	}
	zb_free_buf(buf);
}

static void bulb_turn_on(void)
{
	bulb_state = 1;
	LED_set_color_HEX(colors[bulb_cur_color]);
}

static void bulb_turn_off(void)
{
	bulb_state = 0;
	LED_set_color_HEX(0);
}

static void bulb_toggle_color(void)
{
	bulb_cur_color++;
	if(bulb_cur_color >= sizeof(colors)/sizeof(zb_uint32_t))
	{
		bulb_cur_color = 0;
	}
	LED_set_color_HEX(colors[bulb_cur_color]);
}

static void bulb_toggle(void)
{
	if(bulb_state == 0)
	{
		bulb_state = 1;
		LED_set_color_HEX(colors[bulb_cur_color]);
	}
	else
	{
		bulb_state = 0;
		LED_set_color_HEX(0);
	}	
}

static void bulb_brightness_up(void)
{
	if(bulb_intensity < INTENSITY_STEP_COUNT)
	{
		bulb_intensity++;
	}
	else
	{
		bulb_intensity = 0;
	}
	LED_set_intensity(bulb_intensity);
}

static void bulb_brightness_down(void)
{
	if(bulb_intensity != 0)
	{
		bulb_intensity--;
	}	
	else 
	{
		bulb_intensity = INTENSITY_STEP_COUNT;
	}
	LED_set_intensity(bulb_intensity);	
}

static void bulb_brightness_level(bulb_brightness_t *bulb_brightness)
{
	bulb_intensity = bulb_brightness->brightness;
	LED_set_intensity(bulb_intensity);		
}