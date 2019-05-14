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

#include "buttons.h"
#include "bulb_send.h"

#define BULB_ADDRESS 0x0000

/*! \addtogroup ZB_TESTS */
/*! @{ */

void buttons_scan_cb(zb_uint8_t param) ZB_CALLBACK;
void buttons_click_handle_cb(zb_uint8_t param) ZB_CALLBACK;
void send_command_toggle(zb_uint8_t param) ZB_CALLBACK;
void send_command_color_toggle(zb_uint8_t param) ZB_CALLBACK;
void send_command_step_up(zb_uint8_t param) ZB_CALLBACK;
static void init_btn(void);

typedef enum
{
    BUTTON_BOARD = 0,
    BUTTON_LEFT,
    BUTTON_RIGHT
} Buttons;

zb_uint16_t bulb_address = BULB_ADDRESS;

zb_ieee_addr_t g_zr_addr = {0x01, 0x8D, 0x00, 0xad, 0xde, 0x00, 0xce, 0xfa};

MAIN()
{
    ARGV_UNUSED;

#if !(defined KEIL || defined SDCC || defined ZB_IAR)
    if (argc < 3)
    {
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

    init_btn();

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
        ZB_SCHEDULE_CALLBACK(buttons_scan_cb, 0);
    }
    else
    {
        TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d",
                  (FMT__D, (int)buf->u.hdr.status));
        zb_free_buf(buf);
    }
}

void buttons_click_handle_cb(zb_uint8_t param) ZB_CALLBACK
{
    ZVUNUSED(param);
    static uint8_t prev_cnt = 0;
    if (button_on_press_count() == 0)
    {
        prev_cnt = 0;
    }
    else if (button_on_press_count() == 2)
    {
        prev_cnt = 2;
        if (button_with_repeat(BUTTON_RIGHT))
        {
            ZB_GET_OUT_BUF_DELAYED(send_command_color_toggle);
        }
    }
    if (prev_cnt != 2)
    {
        if (button_on_hold(BUTTON_LEFT) && button_with_repeat(BUTTON_LEFT))
        {
            ZB_GET_OUT_BUF_DELAYED(send_command_step_up);
        }
        else if (button_on_hold(BUTTON_RIGHT) && button_pressed(BUTTON_RIGHT))
        {
            ZB_GET_OUT_BUF_DELAYED(send_command_toggle);
        }
    }
}

static void init_btn(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    /* Init LEDs on discovery board */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin =
        GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_ResetBits(LED_PORT, LED_FIRST_PIN << 0 | LED_FIRST_PIN << 1 |
                                 LED_FIRST_PIN << 2 | LED_FIRST_PIN << 3);

    /// The order matches the names of the buttons
    static uint16_t pins[] = {GPIO_Pin_0, GPIO_Pin_0, GPIO_Pin_1};
    static GPIO_TypeDef *ports[] = {GPIOA, GPIOE, GPIOE};
    static GPIOPuPd_TypeDef pupd[] = {GPIO_PuPd_DOWN, GPIO_PuPd_UP,
                                      GPIO_PuPd_UP};
    static buttons_phys_t buttons = {pins, ports, pupd};
    buttons_init(3, &buttons);
    uint32_t rcc[] = {RCC_AHB1Periph_GPIOA, RCC_AHB1Periph_GPIOE};
    buttons_periph_init(2, rcc);
}

void buttons_scan_cb(zb_uint8_t param) ZB_CALLBACK
{
    ZVUNUSED(param);
    if (buttons_irq())
    {
        ZB_SCHEDULE_CALLBACK(buttons_click_handle_cb, 0);
    }
    ZB_SCHEDULE_ALARM(buttons_scan_cb, 0, 1);
}

void send_command_toggle(zb_uint8_t param) ZB_CALLBACK
{
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
    zb_uint16_t *bulb_addr = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));
    *bulb_addr = bulb_address;
    ZB_SCHEDULE_CALLBACK(send_bulb_toggle, param);
}

void send_command_step_up(zb_uint8_t param) ZB_CALLBACK
{
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
    zb_uint16_t *bulb_addr = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));
    *bulb_addr = bulb_address;
    ZB_SCHEDULE_CALLBACK(send_bulb_brightness_step_up, param);
}

void send_command_color_toggle(zb_uint8_t param) ZB_CALLBACK
{
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
    zb_uint16_t *bulb_addr = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));
    *bulb_addr = bulb_address;
    ZB_SCHEDULE_CALLBACK(send_bulb_color_toggle, param);
}