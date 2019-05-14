#include "bulb_receive.h"

static void process_command(zb_buf_t *buf);
static void bulb_turn_on(void);
static void bulb_turn_off(void);
static void bulb_toggle(void);
static void bulb_toggle_color(void);
static void bulb_brightness_up(void);
static void bulb_brightness_down(void);
static void bulb_brightness_level(bulb_brightness_t *bulb_brightness);

static zb_uint32_t colors[] = {0xFF0000, 0xFFFF00, 0x00FF00,
                               0x00FFFF, 0x0000FF, 0xFF00FF};
static zb_uint8_t bulb_state = BULB_OFF;
static zb_uint8_t bulb_cur_color = 0;
static zb_uint16_t bulb_intensity = 6;

void bulb_get_data(zb_uint8_t param) ZB_CALLBACK
{
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
    zb_uint8_t len;

    /* Remove APS header from the packet */
    ZB_APS_HDR_CUT(buf);    
    len = ZB_BUF_LEN(buf);
    
    if (len == 0)
    {
        TRACE_MSG(TRACE_APS2, "### Buffer is empty!!!", (FMT__0));
    }
    else if (len < sizeof(bulb_cmd_t))
    {
        TRACE_MSG(TRACE_APS2, "### Wrong command size in buffer!", (FMT__0));
    }
    else
    {
        process_command(buf);      
    }
    zb_free_buf(buf); 
}

static void process_command(zb_buf_t *buf)
{
    bulb_cmd_t *bulb_cmd;
    bulb_brightness_t *bulb_brightness;
    
    bulb_cmd = (bulb_cmd_t *)ZB_BUF_BEGIN(buf);
    switch (*bulb_cmd)
    {
    case BULB_ON:
        TRACE_MSG(TRACE_APS2, "### Received command ON", (FMT__0));
        bulb_turn_on();
        break;
    case BULB_OFF:
        TRACE_MSG(TRACE_APS2, "### Received command OFF", (FMT__0));
        bulb_turn_off();
        break;
    case BULB_COLOR_TOGGLE:
        TRACE_MSG(TRACE_APS2, "### Received command TOGGLE COLOR",
                  (FMT__0));
        bulb_toggle_color();
        break;
    case BULB_TOGGLE:
        TRACE_MSG(TRACE_APS2, "### Received command TOGGLE", (FMT__0));
        bulb_toggle();
        break;
    case BULB_BRIGHTNESS_STEP_UP:
        TRACE_MSG(TRACE_APS2, "### Received command STEP UP", (FMT__0));
        bulb_brightness_up();
        break;
    case BULB_BRIGHTNESS_STEP_DOWN:
        TRACE_MSG(TRACE_APS2, "### Received command STEP DOWN", (FMT__0));
        bulb_brightness_down();
        break;
    case BULB_BRIGHTNESS_LEVEL:
        if (ZB_BUF_LEN(buf) != sizeof(bulb_brightness_t))
        {
            TRACE_MSG(TRACE_APS2,
                      "### Error: received command SET LEVEL, but "
                      "brightness level has wrong size",
                      (FMT__0));
        }
        else
        {
            bulb_brightness = (bulb_brightness_t *)ZB_BUF_BEGIN(buf);
            TRACE_MSG(TRACE_APS2,
                      "### Received command SET LEVEL, brightness = %d",
                      (FMT__D, bulb_brightness->brightness));
            bulb_brightness_level(bulb_brightness);
        }
        break;
    default:
        TRACE_MSG(TRACE_APS2, "### Received unknown command", (FMT__0));
        break;
    }
}

static void bulb_turn_on(void)
{
    bulb_state = 1;
    LED_set_color_HEX(colors[bulb_cur_color]);
}

static void bulb_turn_off(void)
{
    bulb_state = 0;
    LED_set_color_HEX(0);
}

static void bulb_toggle_color(void)
{
    zb_uint8_t max_colors = sizeof(colors) / sizeof(zb_uint32_t);
    bulb_cur_color = (bulb_cur_color + 1) % max_colors;
    LED_set_color_HEX(colors[bulb_cur_color]);
}

static void bulb_toggle(void)
{
   bulb_state ^= 1;
   LED_set_intensity(bulb_state ? bulb_intensity : 0);
}

static void bulb_brightness_up(void)
{
    bulb_intensity = (bulb_intensity + 1) % (INTENSITY_STEP_COUNT + 1);
    LED_set_intensity(bulb_intensity);
}

static void bulb_brightness_down(void)
{
    bulb_intensity = (bulb_intensity + INTENSITY_STEP_COUNT) % (INTENSITY_STEP_COUNT + 1);
    LED_set_intensity(bulb_intensity);
}

static void bulb_brightness_level(bulb_brightness_t *bulb_brightness)
{
    bulb_intensity = (bulb_brightness->brightness > INTENSITY_STEP_COUNT) 
                     ? INTENSITY_STEP_COUNT : bulb_brightness->brightness;
    LED_set_intensity(bulb_intensity);
}