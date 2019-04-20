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
#include "led.h"
#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"


#define ZB_TEST_DUMMY_DATA_SIZE 10

zb_ieee_addr_t g_zc_addr = {0x00, 0x00, 0xee, 0xff, 0xc0, 0x00, 0xef, 0xbe};

/*! \addtogroup ZB_TESTS */
/*! @{ */

#ifndef ZB_COORDINATOR_ROLE
#error Coordinator role is not compiled!
#endif

/*
  The test is: ZC starts PAN, ZR joins to it by association and send APS data packet, when ZC
  received packet, it sends packet to ZR, when ZR received packet, it sends
  packet to ZC etc.
 */

//static void zc_send_data(zb_buf_t *buf, zb_uint16_t addr);
static void bulb_turn_on(void);
static void bulb_turn_off(void);
static void bulb_toggle(void);
static void bulb_toggle_color(void);
static void bulb_brightness_up(void);
static void bulb_brightness_down(void);
static void bulb_brightness_level(bulb_brightness_t *bulb_brightness);


void zc_get_command(zb_uint8_t param) ZB_CALLBACK;

static zb_uint32_t colors[6] = {0xFF0000, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0xFF00FF};
static zb_uint8_t bulb_state;
static zb_uint8_t bulb_cur_color;
static zb_uint16_t bulb_intensity;

/*test*/
#include "test.c"
/*test*/

MAIN()
{
  ARGV_UNUSED;

#if !(defined KEIL || defined SDCC || defined ZB_IAR)
  if ( argc < 3 )
  {
    //printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif


  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_zc", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zc", "1", "1");
#endif
#ifdef ZB_SECURITY
  ZG->nwk.nib.security_level = 0;
#endif
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_zc_addr);
  MAC_PIB().mac_pan_id = 0x1aaa;

  /* let's always be coordinator */
  ZB_AIB().aps_designated_coordinator = 1;
  ZB_AIB().aps_channel_mask = (1l << 19);

#ifdef TESTING
	test_intens_led();/*for test!!!*/
#endif
	
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
  TRACE_MSG(TRACE_APS3, ">>zb_zdo_startup_complete status %d", (FMT__D, (int)buf->u.hdr.status));
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
		LED_init_periph();
		bulb_state = 0;
		bulb_cur_color = 0;
		bulb_intensity = 120;
		zb_af_set_data_indication(zc_get_command);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device start FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
  }
  zb_free_buf(buf);
}


/*
   Trivial test: dump all APS data received
 */


void zc_get_command(zb_uint8_t param) ZB_CALLBACK
{
	zb_uint8_t *ptr;
	zb_uint8_t len;
	bulb_cmd_t *bulb_cmd;
	
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
				bulb_turn_on();
				break;
			case BULB_OFF:
				bulb_turn_off();
				break;
			case BULB_COLOR_TOGGLE:
				bulb_toggle_color();
				break;
			case BULB_TOGGLE:
				bulb_toggle();
				break;	
			case BULB_BRIGHTNESS_STEP_UP:
				bulb_brightness_up();
				break;
			case BULB_BRIGHTNESS_STEP_DOWN:
				bulb_brightness_down();
				break;
			case BULB_BRIGHTNESS_LEVEL:
				if(len != sizeof(bulb_brightness_t))
				{
					TRACE_MSG(TRACE_APS2, "### Error: received command SET LEVEL, but brightness level has wrong size", (FMT__0));	 
				}
				else
				{
					bulb_brightness_level((bulb_brightness_t *)ZB_BUF_BEGIN(buf));
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
	if(bulb_cur_color > sizeof(colors)/sizeof(zb_uint32_t))
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
	bulb_intensity += 4;
	LED_set_intensity(bulb_intensity);	
}
static void bulb_brightness_down(void)
{
	TRACE_MSG(TRACE_APS2, "### Received command STEP DOWN", (FMT__0));
	bulb_intensity -= 8;
	LED_set_intensity(bulb_intensity);		
}

static void bulb_brightness_level(bulb_brightness_t *bulb_brightness)
{
	TRACE_MSG(TRACE_APS2, "### Received command SET LEVEL, brightness = %d", (FMT__D, bulb_brightness->brightness));	 
	bulb_intensity = bulb_brightness->brightness;
	LED_set_intensity(bulb_intensity);		

}

/*! @} */
