#ifndef BULB_RECEIVE_H
#define BULB_RECEIVE_H

#include "zb_common.h"
#include "zb_aps.h"

#include "bulb_data_types.h"
#include "buttons.h"
#include "globals.h"
#include "led.h"

//#define TEST_BTN

void bulb_get_data(zb_uint8_t param) ZB_CALLBACK;

#ifdef TEST_BTN
void init_btn(void);
void buttons_scan_cb(zb_uint8_t param) ZB_CALLBACK;
void buttons_click_test_cb(zb_uint8_t param) ZB_CALLBACK;
#endif

#endif /*BULB_RECEIVE_H*/