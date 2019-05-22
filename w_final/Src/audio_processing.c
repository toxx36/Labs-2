#include <audio_processing.h>
#include "arm_math.h"
#include "math.h"

static void gain_calc_freqs(void);
/*static void filter_gain(float32_t *data, size_t data_size);*/
static void filter_simple(float32_t *data_out, float32_t data_in, float32_t coef);//size_t count, float32_t coef)
static void filter_get_max(float32_t *data, size_t data_size);
static void filter_calc(size_t data_size);
static void count_MA(float32_t *data, size_t data_size, size_t window_size);
static float32_t* audio_fft(void);
static float32_t* audio_input_conversion(uint16_t *audio);
static void visual_get_peaks(void);

static float32_t avg_coef = FILTER_AVG_FREQ_SPEED;
static float32_t avg_mult = FILTER_AVG_MULT;
static float32_t max_coef = FILTER_MAX_FREQ_SPEED;
static float32_t max_min_dif = FILTER_MAX_FREQ_MIN_DIF;
static float32_t gain_coef = GAIN_COEF;
static float32_t left_ch_buf[FFT_COMPLEX_SIZE];
static float32_t magn[MAG_SIZE] = {0};
static float32_t filter_data[MAG_SIZE] = {0};
static uint16_t bins_division[LED_COUNT + 1] = {LED1_RANGE, LED2_RANGE, LED3_RANGE, LED4_RANGE, MAG_SIZE};
static float32_t max_freqs_power[LED_COUNT];
static float32_t avg_freqs_power[LED_COUNT];
static float32_t gain_freqs_power[LED_COUNT];

/*
static void filter_gain(float32_t *data, size_t data_size)
{
	size_t i;
	for(i = 0; i < data_size; i++)
		if(data[i] <= filter_data[i])
		{
			data[i] = 0;
		}
		else
		{
			data[i] -= filter_data[i];
		}
}
*/

static void filter_simple(float32_t *data_out, float32_t data_in, float32_t coef)//size_t count, float32_t coef)
{
	*data_out = data_in * coef + *data_out * (1 - coef);
}

static void filter_get_max(float32_t *data, size_t data_size)
{
	size_t i;
	for(i = 0; i < data_size; i++)
	{
		if(data[i] > filter_data[i])
		{
			filter_data[i] = data[i];
		}
	}
}

static void filter_calc(size_t data_size)
{
	count_MA(filter_data, data_size, 3);
}

static void gain_calc_freqs(void)
{
	size_t i;
	for(i = 0; i < LED_COUNT; i++)
	{	/* calc avg value of gain */
		arm_mean_f32(filter_data + bins_division[i], bins_division[i + 1] - bins_division[i], gain_freqs_power + i);
	}
}

static void count_MA(float32_t *data, size_t data_size, size_t window_size)
{
	size_t i; /* data counter */
	size_t j = 0; /* window counter */
	float32_t window_sum = 0;
	float32_t window[MA_MAX_WINDOW_SIZE] = {0};
	if(window_size > MA_MAX_WINDOW_SIZE)
	{
		window_size = MA_MAX_WINDOW_SIZE;
	}
	for(i = 0; i < data_size; i++)
	{
		window_sum -= window[j];
		window[j] = data[i];
		window_sum += window[j];
		j = (j + 1) % window_size;
		/* first (window_size - 1) elements divided by (i + 1) */
		data[i] = window_sum / ((i < window_size - 1) ? (i + 1) : window_size);
	}
}

static float32_t* audio_fft(void)
{
	arm_rfft_fast_instance_f32 fft_instance;
	float32_t output[FFT_SIZE] = {0};
	arm_rfft_fast_init_f32(&fft_instance, FFT_SIZE/*bin count*/);
	arm_rfft_fast_f32(&fft_instance, left_ch_buf, output, 0);
	/* ignore 2 first Re values in output - constant values*/
	arm_cmplx_mag_f32(output + 2, magn, MAG_SIZE);
	return magn;
}

static float32_t* audio_input_conversion(uint16_t *audio)
{
	size_t i;
	for(i = 0; i < BUF_SIZE; i++)
	{
		left_ch_buf[i*2] = (int32_t)(audio[i*4] << 16 | audio[i*4+1]) >> 8;
		left_ch_buf[i*2 + 1] = 0;
	}
	return left_ch_buf;
}

static void visual_get_peaks(void)
{
	size_t i;
	float32_t mean;
	float32_t freq_power;
	float32_t avg_cur;
	uint32_t max_index;
	for(i = 0; i < LED_COUNT; i++)
	{	/* calc avg and max value freq interval */
		arm_mean_f32(magn + bins_division[i], bins_division[i + 1] - bins_division[i], &mean);
		filter_simple(avg_freqs_power + i, mean, avg_coef);
		avg_cur = avg_freqs_power[i] * avg_mult; /* move up avg line */
		arm_max_f32(magn + bins_division[i], bins_division[i + 1] - bins_division[i], &freq_power, &max_index);
		if(freq_power > max_freqs_power[i] && freq_power > avg_cur
				&& freq_power / avg_freqs_power[i] > max_min_dif
				&& avg_freqs_power[i] > gain_freqs_power[i] * gain_coef)
		{
			max_freqs_power[i] = freq_power;
			led_set_max(i + 1);
		}
		else
		{
			filter_simple(max_freqs_power + i, freq_power, max_coef);
			led_fade_out(i + 1, LED_FADE_SPEED);
		}
	}
}

void audio_I2S_handler(uint16_t *buffer16)
{
	static uint8_t state_counter = 0;
	static uint16_t filter_counter = 0;
	if(state_counter > MIC_IDLE_TIME)
	{
		audio_input_conversion(buffer16);
		audio_fft();
		visual_get_peaks();
	}
	else if(state_counter < MIC_IDLE_TIME)
	{
		state_counter++;
	}
	else if(state_counter == MIC_IDLE_TIME)
	{
		if(filter_counter < GAIN_SIZE)
		{
			audio_input_conversion(buffer16);
			filter_get_max(audio_fft(), MAG_SIZE);
			filter_counter++;
		}
		else
		{
			filter_calc(MAG_SIZE);
			gain_calc_freqs();
			state_counter++;
		}
	}
}
