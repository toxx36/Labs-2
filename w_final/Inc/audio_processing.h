#ifndef AUDIO_PROCESSING_H_
#define AUDIO_PROCESSING_H_

#include "main.h"
#include "leds.h"

/* Mic params */
#define CHANNEL_COUNT 1
#define FIRST_CHANNEL 0
#define MIC_IDLE_TIME 20

/* FFT params */
#define FFT_SIZE 1024
#define BUF_SIZE FFT_SIZE
#define SAMPLES_SIZE (FFT_SIZE * 2 /*channels*/ * 2 /*2 uint16 in int32*/)
#define DMA_BUF_SIZE (SAMPLES_SIZE * 2) /* double buffering */
#define FFT_COMPLEX_SIZE (FFT_SIZE * 2)
#define MAG_SIZE (FFT_SIZE / 4)

/* Moving average max size */
#define MA_MAX_WINDOW_SIZE 20

/* LED frequencies distribution */
#define LED1_RANGE 0
#define LED2_RANGE (LED3_RANGE/4)
#define LED3_RANGE (LED4_RANGE/4)
#define LED4_RANGE (MAG_SIZE/4)

/* Gain exposure in spectrums count */
#define GAIN_SIZE 300

/* Visual peaks params */
#define GAIN_COEF 0.4
#define FILTER_AVG_FREQ_SPEED 0.05
#define FILTER_AVG_MULT 2.0
#define FILTER_MAX_FREQ_SPEED 0.08
#define FILTER_MAX_FREQ_MIN_DIF 1.2

/* Visual power params */
#define GAIN_MAX_COEF 1.2
#define GAIN_MIN_COEF 0.5
#define FILTER_MAX_LEVEL_UP_SPEED 0.3
#define FILTER_MAX_LEVEL_DOWN_SPEED 0.01
#define FILTER_MAX_LEVEL_ADDITION 7.0

#define BUTTON_HOLD_TIME 30

/* etc */
#define LOG255 2.4065401554107666015625
#define AUDIO_FREQ_F 22058.5

void audio_I2S_handler(uint16_t *buffer16);

#endif /* AUDIO_PROCESSING_H_ */
