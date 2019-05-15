#ifndef BULB_SEND_H
#define BULB_SEND_H

#include "zb_common.h"
#include "zb_aps.h"

#include "globals.h"
#include "bulb_data_types.h"

/*! 
 *  \brief  Sends over ZigBee "BULB_TURN_ON" command
 *  \params param ZBOSS buffer with an address of the bulb at the tail
 */
void send_bulb_state_on(zb_uint8_t param) ZB_CALLBACK;

/*! 
 *  \brief  Sends over ZigBee "BULB_TURN_OFF" command
 *  \params param ZBOSS buffer with an address of the bulb at the tail
 */
void send_bulb_state_off(zb_uint8_t param) ZB_CALLBACK;

/*! 
 *  \brief  Sends over ZigBee "BULB_TOGGLE" command
 *  \params param ZBOSS buffer with an address of the bulb at the tail
 */
void send_bulb_toggle(zb_uint8_t param) ZB_CALLBACK;

/*! 
 *  \brief  Sends over ZigBee "BULB_COLOR_TOGGLE" command
 *  \params param ZBOSS buffer with an address of the bulb at the tail
 */
void send_bulb_color_toggle(zb_uint8_t param) ZB_CALLBACK;

/*! 
 *  \brief  Sends over ZigBee "BULB_BRIGHTNESS_STEP_UP" command
 *  \params param ZBOSS buffer with an address of the bulb at the tail
 */
void send_bulb_brightness_step_up(zb_uint8_t param) ZB_CALLBACK;

/*! 
 *  \brief  Sends over ZigBee "BULB_BRIGHTNESS_STEP_DOWN" command
 *  \params param ZBOSS buffer with an address of the bulb at the tail
 */
void send_bulb_brightness_step_down(zb_uint8_t param) ZB_CALLBACK;

/*! 
 *  \brief  Sends over ZigBee "BULB_BRIGHTNESS_LEVEL" command with brightness level
 *  \params param ZBOSS buffer with an address of the bulb and the level of brithtness (bulb_send_brightness_t type) at the tail
 */
void send_bulb_brightness_level(zb_uint8_t param) ZB_CALLBACK;

#endif /*BULB_SEND_H*/