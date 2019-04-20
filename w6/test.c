#define TESTING

void inc_intens(zb_uint8_t param) ZB_CALLBACK
{
	ZVUNUSED(param);
	bulb_intensity++;
	if(bulb_intensity > 10) bulb_intensity = 0;
	LED_set_intensity(bulb_intensity);
	ZB_SCHEDULE_ALARM(inc_intens, 0, ZB_TIME_ONE_SECOND/2);
}
void fake_commands(zb_uint8_t param) ZB_CALLBACK
{
	ZVUNUSED(param);
	static bulb_cmd_t bulb_cmd = BULB_ON;
	bulb_brightness_t brght = {0, 6};
	switch(bulb_cmd) 
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
			bulb_brightness_level(&brght);
			break;
		default:
			TRACE_MSG(TRACE_APS2, "### Received unknown command", (FMT__0));	
		break;
	}
	bulb_cmd++;
	if(bulb_cmd > BULB_BRIGHTNESS_LEVEL)
	{
		bulb_cmd = BULB_ON;
	}
	ZB_SCHEDULE_ALARM(fake_commands, 0, ZB_TIME_ONE_SECOND/2);
}
static void test_intens_led(void)
{
	LED_init_periph();
	bulb_intensity = 6;
	bulb_state = 0;
	bulb_cur_color = 0;
//	TIM_SetCompare1(TIM1,10);
//	ZB_SCHEDULE_ALARM(inc_intens, 0, ZB_TIME_ONE_SECOND/2);
	ZB_SCHEDULE_ALARM(fake_commands, 0, ZB_TIME_ONE_SECOND);
	zdo_main_loop();
}