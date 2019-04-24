#ifndef BULB_REMOTE_H
#define BULB_REMOTE_H

#include "globals.h"
//#include "zb_types.h"
#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
//#include "zb_secur.h"
//#include "zb_secur_api.h"
#include "bulb_data_types.h"
#include "buttons.h"


//void next_command(zb_uint8_t param) ZB_CALLBACK;
extern bulb_cmd_t remote_command;
extern zb_uint8_t demo_level;
extern uint16_t bulb_address;

void send_command(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_state_on(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_state_off(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_toggle(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_color_toggle(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_brightness_step_up(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_brightness_step_down(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_brightness_level(zb_uint8_t param) ZB_CALLBACK;

void button_bulb(zb_uint8_t param) ZB_CALLBACK;
void btn_IRQ_cb(zb_uint8_t param) ZB_CALLBACK;
void init_btn(void);

#endif /*BULB_REMOTE_H*/