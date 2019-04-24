#include "bulb_input.h"

static void bulb_turn_on(void);
static void bulb_turn_off(void);
static void bulb_toggle(void);
static void bulb_toggle_color(void);
static void bulb_brightness_up(void);
static void bulb_brightness_down(void);
static void bulb_brightness_level(bulb_brightness_t *bulb_brightness);

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

	///The order matches the names of the buttons
	uint16_t pins[] = {GPIO_Pin_0, GPIO_Pin_0, GPIO_Pin_1};
	GPIO_TypeDef *ports[] = {GPIOA, GPIOE, GPIOE}; 
	GPIOPuPd_TypeDef pupd[] = {GPIO_PuPd_DOWN, GPIO_PuPd_UP, GPIO_PuPd_UP};
	buttons_init_t buttons = {pins, ports, pupd};
	uint32_t rcc[] = {RCC_AHB1Periph_GPIOA, RCC_AHB1Periph_GPIOE};	
	ButtonInit(3, &buttons, 2, rcc);


	GPIO_ResetBits(LED_PORT,LED_FIRST_PIN<<0|LED_FIRST_PIN<<1|LED_FIRST_PIN<<2|LED_FIRST_PIN<<3);
	
}

void button_bulb(zb_uint8_t param) ZB_CALLBACK
{
	ZVUNUSED(param);
	bulb_brightness_t brght = {0};
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
				bulb_intensity++;
				if(bulb_intensity > 10)
				{
					bulb_intensity = 0;
				}
				brght.brightness = bulb_intensity;
				bulb_brightness_level(&brght);
			}
			else if (Button_onHold(bRight) && Button_pressed(bRight))
			{
				bulb_toggle();
			}
		}
		
//	ZB_SCHEDULE_ALARM(button_bulb, 0, ZB_TIME_ONE_SECOND/10);
}


void btn_IRQ_cb(zb_uint8_t param) ZB_CALLBACK
{
	ZVUNUSED(param);
	if(Buttons_IRQ() > 0)
	{
		GPIO_ResetBits(LED_PORT,LED_FIRST_PIN<<2|LED_FIRST_PIN<<3);

		ZB_SCHEDULE_CALLBACK(button_bulb, 0);
	}
	ZB_SCHEDULE_ALARM(btn_IRQ_cb, 0, 1);
}


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
	TRACE_MSG(TRACE_APS2, "### Received command ON", (FMT__0));	
	bulb_state = 1;
	LED_set_color_HEX(colors[bulb_cur_color]);
}
static void bulb_turn_off(void)
{
	TRACE_MSG(TRACE_APS2, "### Received command OFF", (FMT__0));	
	bulb_state = 0;
	LED_set_color_HEX(0);
}
static void bulb_toggle_color(void)
{
	TRACE_MSG(TRACE_APS2, "### Received command TOGGLE COLOR", (FMT__0));	
	bulb_cur_color++;
	if(bulb_cur_color >= sizeof(colors)/sizeof(zb_uint32_t))
	{
		bulb_cur_color = 0;
	}
	LED_set_color_HEX(colors[bulb_cur_color]);
}

static void bulb_toggle(void)
{
	TRACE_MSG(TRACE_APS2, "### Received command TOGGLE", (FMT__0));	
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
	TRACE_MSG(TRACE_APS2, "### Received command STEP UP", (FMT__0));
	if(bulb_intensity < MAX_INTENSITY)
	{
		bulb_intensity++;
		LED_set_intensity(bulb_intensity);
	}
}
static void bulb_brightness_down(void)
{
	TRACE_MSG(TRACE_APS2, "### Received command STEP DOWN", (FMT__0));
	if(bulb_intensity != 0)
	{
		bulb_intensity--;
		LED_set_intensity(bulb_intensity);
	}		
}

static void bulb_brightness_level(bulb_brightness_t *bulb_brightness)
{
	TRACE_MSG(TRACE_APS2, "### Received command SET LEVEL, brightness = %d", (FMT__D, bulb_brightness->brightness));	 
	bulb_intensity = bulb_brightness->brightness;
	LED_set_intensity(bulb_intensity);		

}