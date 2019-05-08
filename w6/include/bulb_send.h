#ifndef BULB_SEND_H
#define BULB_SEND_H

#include "globals.h"
#include "zb_common.h"
#include "zb_aps.h"
#include "bulb_data_types.h"

void send_bulb_state_on(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_state_off(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_toggle(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_color_toggle(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_brightness_step_up(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_brightness_step_down(zb_uint8_t param) ZB_CALLBACK;
void send_bulb_brightness_level(zb_uint8_t param) ZB_CALLBACK;

#endif /*BULB_SEND_H*/