#ifndef MQ9_H_
#define MQ9_H_

#include "driver/adc.h"

#define RATIO_MQ9_CLEAN_AIR 9.6   // RS/R0 = 9.6 
#define VOLTAGE_RANGE 3.3         // ESP32 ADC voltage range
#define DEFAULT_VREF_MQ9 3.3      // ADC reference voltage         

static const adc_channel_t channel = ADC_CHANNEL_5;     // GPIO33 if ADC1
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

typedef struct {
    adc1_channel_t channel;
    adc_bits_width_t width;
    adc_atten_t atten;
    float a;
    float b;
    float rl;
    float r0;
} mq9_t;

void mq9_init(mq9_t *data, adc_channel_t channel, adc_bits_width_t width, adc_atten_t atten, float a, float b, float rl);
float mq9_read_ratio(mq9_t *data);
float mq9_read(mq9_t *data, float ratio);

#endif // MQ9_H