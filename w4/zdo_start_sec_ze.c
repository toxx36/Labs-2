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
#include "zb_secur.h"
#include "zb_secur_api.h"
#include "bulb_data_types.h"

#ifndef ZB_ED_ROLE
#error define ZB_ED_ROLE to compile ze tests
#endif
/*! \addtogroup ZB_TESTS */
/*! @{ */
#ifndef ZB_SECURITY
#error Define ZB_SECURITY
#endif

#define BULB_ADDRESS 0x0000
#define COMMAND_COUNT 6

/*! \addtogroup ZB_TESTS */
/*! @{ */

zb_uint8_t demo_level = 0;

zb_uint8_t zr_remote_command;
zb_uint8_t new_buffer;
zb_uint8_t Command_send(zb_uint8_t command);

void schedule_send_command(zb_uint8_t param, bulb_cmd_t command);

void send_bulb_state_on(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_state_off(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_toggle(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_color_toggle(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_brightness_step_up(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_brightness_step_down(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_brightness_level(zb_uint8_t param) ZB_CALLBACK;

void send_data_by_addr(zb_uint8_t param) ZB_CALLBACK;
void next_command(zb_uint8_t param) ZB_CALLBACK;
void send_command(zb_uint8_t param) ZB_CALLBACK;
 
/*
	ZE joins to ZC(ZR), then sends APS packet.
*/

zb_ieee_addr_t g_ieee_addr = {0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

MAIN()
{
	ARGV_UNUSED;

#ifndef KEIL
	if ( argc < 3 )
	{
		printf("%s <read pipe path> <write pipe path>\n", argv[0]);
		return 0;
	}
#endif

	/* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
	ZB_INIT("zdo_ze", argv[1], argv[2]);
#else
	ZB_INIT("zdo_ze", "2", "2");
#endif

	ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), g_ieee_addr);

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
		zr_remote_command = 0;
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
	bulb_data_t *bulb_data = ZB_GET_BUF_TAIL(buf, sizeof(bulb_data_t));
	bulb_data->address = BULB_ADDRESS;
	switch(zr_remote_command) {
		case 0:
		ZB_SCHEDULE_CALLBACK(send_bulb_state_off,ZB_REF_FROM_BUF(buf));
		break;
		case 1:
		ZB_SCHEDULE_CALLBACK(send_bulb_state_on,ZB_REF_FROM_BUF(buf));
		break;
		case 2:
		ZB_SCHEDULE_CALLBACK(send_bulb_toggle,ZB_REF_FROM_BUF(buf));
		break;
		case 3:
		ZB_SCHEDULE_CALLBACK(send_bulb_color_toggle,ZB_REF_FROM_BUF(buf));
		break;
		case 4:
		ZB_SCHEDULE_CALLBACK(send_bulb_brightness_step_up,ZB_REF_FROM_BUF(buf));
		break;
		case 5:
		ZB_SCHEDULE_CALLBACK(send_bulb_brightness_step_down,ZB_REF_FROM_BUF(buf));
		break;
		case 6:
		demo_level += 25;
		bulb_data->parameter = demo_level;
		ZB_SCHEDULE_CALLBACK(send_bulb_brightness_level,ZB_REF_FROM_BUF(buf));
		break;
		default:
		break;
	}
}



void next_command(zb_uint8_t param) ZB_CALLBACK ///Generate command and create new buffer
{
		if(++zr_remote_command > COMMAND_COUNT) zr_remote_command = 0;
		ZB_GET_OUT_BUF_DELAYED(send_command);
		ZB_SCHEDULE_ALARM(next_command, 0, ZB_TIME_ONE_SECOND);
}

void send_data_by_addr(zb_uint8_t param) ZB_CALLBACK
{
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	zb_apsde_data_req_t *req;
	zb_uint16_t address = *(uint16_t *)ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));

	req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
	req->dst_addr.addr_short = address; /* send to ZC */
	req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
	req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
	req->radius = 1;
	req->profileid = 2;
	req->src_endpoint = 10;
	req->dst_endpoint = 10;

	buf->u.hdr.handle = 0x11;

	TRACE_MSG(TRACE_APS2, "Sending apsde_data.request", (FMT__0));
	ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}

void send_bulb_state_on(zb_uint8_t param) ZB_CALLBACK
{
	schedule_send_command(param, BULB_ON);
}

void send_bulb_state_off(zb_uint8_t param) ZB_CALLBACK
{
	schedule_send_command(param, BULB_OFF);
}

void send_bulb_toggle(zb_uint8_t param) ZB_CALLBACK
{
	schedule_send_command(param, BULB_TOGGLE);
}

void send_bulb_color_toggle(zb_uint8_t param) ZB_CALLBACK
{
	schedule_send_command(param, BULB_COLOR_TOGGLE);
}

void send_bulb_brightness_step_up(zb_uint8_t param) ZB_CALLBACK
{
	schedule_send_command(param, BULB_BRIGHTNESS_STEP_UP);
}

void send_bulb_brightness_step_down(zb_uint8_t param) ZB_CALLBACK
{
	schedule_send_command(param, BULB_BRIGHTNESS_STEP_DOWN);
}

void send_bulb_brightness_level(zb_uint8_t param) ZB_CALLBACK
{
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	zb_uint16_t *addr_tail;
	zb_uint16_t addr_send;
	bulb_data_t *bulb_data_tail;
/**/bulb_data_cmd_param_t *bulb_data_send;
	
	bulb_data_tail = ZB_GET_BUF_TAIL(buf, sizeof(bulb_data_t));					//get the tail
	addr_send = bulb_data_tail->address; 										//save address from tail
/**/ZB_BUF_INITIAL_ALLOC(buf, sizeof(bulb_data_cmd_param_t), bulb_data_send); 	//create buf with size of the sending data
/**/bulb_data_send->brightness = bulb_data_tail->parameter;						//save parameter to send from tail
/**/bulb_data_send->command = BULB_BRIGHTNESS_LEVEL;							//set command to send
	addr_tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t)); 						//get new tail size for store the address
	*addr_tail = addr_send;														//set address to send

/**/TRACE_MSG(TRACE_APS2, "### Sending command = %d, brightness = %d, address = %d", (FMT__D_D_D, bulb_data_send->command,bulb_data_send->brightness,*addr_tail));
	ZB_SCHEDULE_CALLBACK(send_data_by_addr, ZB_REF_FROM_BUF(buf));
}

void schedule_send_command(zb_uint8_t param, bulb_cmd_t command)
{
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	zb_uint16_t *addr_tail;
	zb_uint16_t addr_send;
	bulb_data_t *bulb_data_tail;
	bulb_cmd_t *bulb_command_send;
	
	bulb_data_tail = ZB_GET_BUF_TAIL(buf, sizeof(bulb_data_t));			//get the tail
	addr_send = bulb_data_tail->address; 								//save address from tail
	ZB_BUF_INITIAL_ALLOC(buf, sizeof(bulb_cmd_t), bulb_command_send); 	//create buf with size of the sending command
	*bulb_command_send = command;										//set command to send
	addr_tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t)); 				//get new tail size for store the address
	*addr_tail = addr_send;												//set address to send

	TRACE_MSG(TRACE_APS2, "### Sending command = %d, address = %d", (FMT__D_D, *bulb_command_send,*addr_tail));
	ZB_SCHEDULE_CALLBACK(send_data_by_addr, ZB_REF_FROM_BUF(buf));
}


/*! @} */