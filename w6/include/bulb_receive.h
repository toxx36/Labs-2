#ifndef BULB_RECEIVE_H
#define BULB_RECEIVE_H

#include "zb_common.h"
#include "zb_aps.h"

#include "bulb_data_types.h"
#include "buttons.h"
#include "globals.h"
#include "led.h"

/*!
 *  /brief Function for receive and process data for the bulb
 *  /param param incoming buffer
 *  Gets command for bulb and performs it. Set it for data_indication.
 */
void bulb_get_data(zb_uint8_t param) ZB_CALLBACK;

#endif /*BULB_RECEIVE_H*/