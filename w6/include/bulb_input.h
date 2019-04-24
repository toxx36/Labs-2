#ifndef BULB_INPUT_H
#define BULB_INPUT_H

#include "globals.h"
//#include "zb_types.h"
#include "zb_common.h"
//#include "zb_scheduler.h"
//#include "zb_bufpool.h"
//#include "zb_nwk.h"
#include "zb_aps.h"
//#include "zb_zdo.h"
//#include "zb_secur.h"
//#include "zb_secur_api.h"
#include "bulb_data_types.h"
#include "led.h"
#include "buttons.h"

extern zb_uint32_t colors[6];
extern zb_uint8_t bulb_state;
extern zb_uint8_t bulb_cur_color;
extern zb_uint16_t bulb_intensity;

void init_btn(void);
void bulb_get_data(zb_uint8_t param) ZB_CALLBACK;
void btn_IRQ_cb(zb_uint8_t param) ZB_CALLBACK;
void button_bulb(zb_uint8_t param) ZB_CALLBACK;

#endif /*BULB_INPUT_H*/