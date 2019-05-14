#ifndef BULB_DATA_TYPES_H
#define BULB_DATA_TYPES_H

typedef enum bulb_cmd_e
{
    BULB_ON = 0,
    BULB_OFF,
    BULB_TOGGLE,
    BULB_COLOR_TOGGLE,
    BULB_BRIGHTNESS_STEP_UP,
    BULB_BRIGHTNESS_STEP_DOWN,
    BULB_BRIGHTNESS_LEVEL
} bulb_cmd_t;

typedef struct bulb_send_brightness_s
{
    zb_uint16_t address;
    zb_uint8_t brightness;
} ZB_PACKED_STRUCT bulb_send_brightness_t;

typedef struct bulb_brightness_s
{
    bulb_cmd_t cmd;
    zb_uint8_t brightness;
} bulb_brightness_t;

#endif
