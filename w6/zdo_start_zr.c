/***************************************************************************
*                      ZBOSS ZigBee Pro 2007 stack                         *
*                                                                          *
*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
*                       http://www.claridy.com/                            *
*                                                                          *
*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
*                             Hsinchu, Taiwan.                             *
*                       http://www.ubec.com.tw/                            *
*                                                                          *
*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*                                                                          *
*                                                                          *
* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
* under either the terms of the Commercial License or the GNU General      *
* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
* may choose which license to receive this code under (except as noted in  *
* per-module LICENSE files).                                               *
*                                                                          *
* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
* Research LLC.                                                            *
*                                                                          *
* GNU General Public License Usage                                         *
* This file may be used under the terms of the GNU General Public License  *
* version 2.0 as published by the Free Software Foundation and appearing   *
* in the file LICENSE.GPL included in the packaging of this file.  Please  *
* review the following information to ensure the GNU General Public        *
* License version 2.0 requirements will be met:                            *
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
*                                                                          *
* Commercial Usage                                                         *
* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
* Agreement provided with the Software or, alternatively, in accordance    *
* with the terms contained in a written agreement between you and          *
* ClarIDy/UBEC/DSR.                                                        *
*                                                                          *
****************************************************************************
PURPOSE: Test for ZC application written using ZDO.
*/


#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

#include "bulb_data_types.h"

#define BULB_ADDRESS 0x0000

/*! \addtogroup ZB_TESTS */
/*! @{ */


zb_uint16_t cur_led;
static void board_init();

#ifndef APS_RETRANSMIT_TEST

#endif

void send_bulb_state_on(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_state_off(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_toggle(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_color_toggle(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_brightness_step_up(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_brightness_step_down(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_brightness_level(zb_uint8_t param) ZB_CALLBACK;

void next_command(zb_uint8_t param) ZB_CALLBACK;

static bulb_cmd_t remote_command = BULB_ON;
static zb_uint8_t demo_level = 0;

static void apsde_send_data(zb_uint8_t param, zb_uint16_t addr_send);
static void send_simple_command(zb_uint8_t param, zb_uint8_t command);
static void send_custom_data(zb_uint8_t param, zb_uint8_t command, zb_uint8_t length, zb_uint_t *ptr);
 


/*
  ZR joins to ZC, then sends APS packet.
 */
 
zb_ieee_addr_t g_zr_addr = {0x01, 0x8D, 0x00, 0xad, 0xde, 0x00, 0xce, 0xfa};

MAIN()
{
  ARGV_UNUSED;

#if !(defined KEIL || defined SDCC|| defined ZB_IAR)
  if ( argc < 3 )
  {
    //printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif

  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_zr", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zr", "2", "2");
#endif
#ifdef ZB_SECURITY
  ZG->nwk.nib.security_level = 0;
#endif

  ZB_AIB().aps_channel_mask = (1l << 19); 
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_zr_addr); 
  board_init();

  if (zdo_dev_start() != RET_OK)
  {
    TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
  }
  else
  {
    zdo_main_loop();
  }

  TRACE_DEINIT();

  MAIN_RETURN(0);
}



void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
	zb_buf_t *buf = ZB_BUF_FROM_REF(param);
	if (buf->u.hdr.status == 0)
	{
		TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
		next_command(0);
	}
	else
	{
		TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
		zb_free_buf(buf);
	}
}

void send_command(zb_uint8_t param) ZB_CALLBACK ///Process the buffer and send command
{

	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	
	if(remote_command == BULB_BRIGHTNESS_LEVEL)
	{
		demo_level += 0x11;
		bulb_send_brightness_t *bulb_data = ZB_GET_BUF_TAIL(buf, sizeof(bulb_send_brightness_t));
		bulb_data->address = BULB_ADDRESS;
		bulb_data->brightness = demo_level;
	}
	else
	{
		zb_uint16_t *bulb_addr = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));
		*bulb_addr = BULB_ADDRESS;
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
	remote_command++;
	if(remote_command > BULB_BRIGHTNESS_LEVEL) remote_command = BULB_ON;

  GPIO_Write(GPIOD, cur_led);
  cur_led <<= 1;
  if(cur_led == 0) cur_led = GPIO_Pin_12;

}

void next_command(zb_uint8_t param) ZB_CALLBACK ///Generate command and create new buffer
{
	ZVUNUSED(param);
	ZB_GET_OUT_BUF_DELAYED(send_command);
	ZB_SCHEDULE_ALARM(next_command, 0, ZB_TIME_ONE_SECOND);
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


static void board_init()
{
	cur_led = GPIO_Pin_12;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable peripheral clock for LEDs port */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  /* Init LEDs */
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  GPIO_SetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
  
}

/*! @} */
