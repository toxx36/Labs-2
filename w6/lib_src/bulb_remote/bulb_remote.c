#include "bulb_remote.h"

static void apsde_send_data(zb_uint8_t param, zb_uint16_t addr_send);
static void send_simple_command(zb_uint8_t param, zb_uint8_t command);
static void send_custom_data(zb_uint8_t param, zb_uint8_t command, zb_uint8_t length, zb_voidp_t ptr);

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
	send_custom_data(param, BULB_BRIGHTNESS_LEVEL, sizeof(tail->brightness), (zb_voidp_t)&(tail->brightness));
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

static void send_custom_data(zb_uint8_t param, zb_uint8_t command, zb_uint8_t length, zb_voidp_t ptr) //b_uint_t *ptr, zb_uint16_t addr)
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