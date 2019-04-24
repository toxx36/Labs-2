#include "bulb_remote.h"

static void apsde_send_data(zb_uint8_t param, zb_uint16_t addr_send);
static void send_simple_command(zb_uint8_t param, zb_uint8_t command);
static void send_custom_data(zb_uint8_t param, zb_uint8_t command, zb_uint8_t length, zb_uint_t *ptr);
void btn_IRQ_cb(zb_uint8_t param) ZB_CALLBACK;
void button_bulb(zb_uint8_t param) ZB_CALLBACK;

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
			remote_command = BULB_COLOR_TOGGLE;
			ZB_GET_OUT_BUF_DELAYED(send_command);
		}
	}
	if(prev_cnt != 2)
	{
		if(Button_onHold(bLeft) && Button_withRepeat(bLeft)) 
		{
			demo_level++;
			if(demo_level > 10)
			{
				demo_level = 0;
			}
			remote_command = BULB_BRIGHTNESS_LEVEL;
			ZB_GET_OUT_BUF_DELAYED(send_command);
		}
		else if (Button_onHold(bRight) && Button_pressed(bRight))
		{
			remote_command = BULB_TOGGLE;
			ZB_GET_OUT_BUF_DELAYED(send_command);
		}
	}
}


void btn_IRQ_cb(zb_uint8_t param) ZB_CALLBACK
{
	ZVUNUSED(param);
	if(Buttons_IRQ())
	{
		ZB_SCHEDULE_CALLBACK(button_bulb, 0);
	}
	ZB_SCHEDULE_ALARM(btn_IRQ_cb, 0, 1);
}


void send_command(zb_uint8_t param) ZB_CALLBACK ///Process the buffer and send command
{

	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	
	if(remote_command == BULB_BRIGHTNESS_LEVEL)
	{
		bulb_send_brightness_t *bulb_data = ZB_GET_BUF_TAIL(buf, sizeof(bulb_send_brightness_t));
		bulb_data->address = bulb_address;
		bulb_data->brightness = demo_level;
	}
	else
	{
		zb_uint16_t *bulb_addr = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));
		*bulb_addr = bulb_address;
	}

	switch(remote_command) {
		case BULB_ON:
		ZB_SCHEDULE_CALLBACK(send_bulb_state_on,ZB_REF_FROM_BUF(buf));
		break;
		case BULB_OFF:
		ZB_SCHEDULE_CALLBACK(send_bulb_state_off,ZB_REF_FROM_BUF(buf));
		break;
		case BULB_TOGGLE:
		ZB_SCHEDULE_CALLBACK(send_bulb_toggle,ZB_REF_FROM_BUF(buf));
		break;
		case BULB_COLOR_TOGGLE:
		ZB_SCHEDULE_CALLBACK(send_bulb_color_toggle,ZB_REF_FROM_BUF(buf));
		break;
		case BULB_BRIGHTNESS_STEP_UP:
		ZB_SCHEDULE_CALLBACK(send_bulb_brightness_step_up,ZB_REF_FROM_BUF(buf));
		break;
		case BULB_BRIGHTNESS_STEP_DOWN:
		ZB_SCHEDULE_CALLBACK(send_bulb_brightness_step_down,ZB_REF_FROM_BUF(buf));
		break;
		case BULB_BRIGHTNESS_LEVEL:
		ZB_SCHEDULE_CALLBACK(send_bulb_brightness_level,ZB_REF_FROM_BUF(buf));
		break;
		default:
		break;
	}
}

void send_bulb_state_on(zb_uint8_t param) ZB_CALLBACK
{
	send_simple_command(param, BULB_ON);
}

void send_bulb_state_off(zb_uint8_t param) ZB_CALLBACK
{
	send_simple_command(param, BULB_OFF);
}

void send_bulb_toggle(zb_uint8_t param) ZB_CALLBACK
{
	send_simple_command(param, BULB_TOGGLE);
}

void send_bulb_color_toggle(zb_uint8_t param) ZB_CALLBACK
{
	send_simple_command(param, BULB_COLOR_TOGGLE);
}

void send_bulb_brightness_step_up(zb_uint8_t param) ZB_CALLBACK
{
	send_simple_command(param, BULB_BRIGHTNESS_STEP_UP);
}

void send_bulb_brightness_step_down(zb_uint8_t param) ZB_CALLBACK
{
	send_simple_command(param, BULB_BRIGHTNESS_STEP_DOWN);
}

void send_bulb_brightness_level(zb_uint8_t param) ZB_CALLBACK
{
	bulb_send_brightness_t *tail = ZB_GET_BUF_TAIL((zb_buf_t *)ZB_BUF_FROM_REF(param), sizeof(bulb_send_brightness_t));
	send_custom_data(param, BULB_BRIGHTNESS_LEVEL, sizeof(tail->brightness), (zb_uint_t*)&(tail->brightness));
}

static void send_simple_command(zb_uint8_t param, zb_uint8_t command)//, zb_uint16_t addr)
{
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	zb_uint16_t addr_send;
	zb_uint16_t *addr_tail;
	bulb_cmd_t *bulb_command_send;

	addr_tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));
	addr_send = *addr_tail; 																					//save address from tail
	ZB_BUF_INITIAL_ALLOC(buf, sizeof(bulb_cmd_t), bulb_command_send); //create buf with size of the sending command
	*bulb_command_send = command;																			//set command to send
	
	TRACE_MSG(TRACE_APS2, "### Sending command = %d to address = %d", (FMT__D_D, *bulb_command_send, addr_send));
	
	apsde_send_data(param, addr_send);
}

static void send_custom_data(zb_uint8_t param, zb_uint8_t command, zb_uint8_t length, zb_uint_t *ptr) //b_uint_t *ptr, zb_uint16_t addr)
{
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	zb_uint16_t addr_send;
	zb_uint16_t *addr_tail;
	bulb_cmd_t *bulb_command_send;

	addr_tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t) + length);
	addr_send = *addr_tail; 																										//save address from tail
	ZB_BUF_INITIAL_ALLOC(buf, sizeof(bulb_cmd_t) + length, bulb_command_send); 	//create new buf with size of the sending command
	*bulb_command_send = command;																								//set command to send
	ZB_MEMCPY(bulb_command_send + 1, ptr, length);	

	TRACE_MSG(TRACE_APS2, "### Sending command = %d to address = %d, payload length = %d", (FMT__D_D_D, *bulb_command_send, addr_send, sizeof(bulb_cmd_t) + length));
	
	apsde_send_data(param, addr_send);
}

static void apsde_send_data(zb_uint8_t param, zb_uint16_t addr_send)
{
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	zb_apsde_data_req_t *req;

	req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
	req->dst_addr.addr_short = addr_send; /* send to ZC */
	req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
	req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
	req->radius = 1;
	req->profileid = 2;
	req->src_endpoint = 10;
	req->dst_endpoint = 10;

	buf->u.hdr.handle = 0x11;

	ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}