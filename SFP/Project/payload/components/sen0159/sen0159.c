#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "esp_log.h"
#include "esp_adc_cal.h"
#include "driver/adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "sen0159.h"

const char *TAG_SEN0159 = "SEN0159";

static esp_adc_cal_characteristics_t adc1_chars;

float CO2Curve[3] = {2.602,ZERO_POINT_VOLTAGE,(REACTION_VOLTAGE/(2.602-3))}; 
        //two points are taken from the curve.
        //with these two points, a line is formed which is
        //"approximately equivalent" to the original curve.
        //data format:{ x, y, slope}; point1: (lg400, 0.324), point2: (lg4000, 0.280)
        //slope = ( reaction voltage ) / (log400 –log1000)

void sen0159Init(sen0159_t *device, adc_channel_t channel, adc_bits_width_t width, adc_atten_t atten){
    ESP_LOGI(TAG_SEN0159, "Initializing ADC");
    esp_adc_cal_characterize(ADC_UNIT_1, atten, width, DEFAULT_VREF_SEN0159, &adc1_chars);
    adc1_config_width(width);
    adc1_config_channel_atten(channel, atten);

    device->channel = channel;
    device->width = width;
    device->atten = atten;
    device->adc_chars = adc1_chars;
}

float sen0159Read(sen0159_t *device){
    int i;
    float voltage = 0;
    int counter = 0;
    for(i = 0; i < NUMBER_SAMPLES; i++){    // take multiple samples and calculate the average value, this happen to be more accurate.
        uint32_t adc_reading = adc1_get_raw(device->channel);                               // read from adc
        float adcVoltage = esp_adc_cal_raw_to_voltage(adc_reading, &device->adc_chars);     // convert adc reading to voltage
        if( adcVoltage == 0){
            ESP_LOGE(TAG_SEN0159, "ADC reading is 0");
        }else if(adcVoltage < ZERO_POINT_VOLTAGE){
            ESP_LOGE(TAG_SEN0159, "Pre-heating is not done");
        }else{
            voltage += adcVoltage;
            counter++;
        }   
        vTaskDelay(READ_SAMPLE_INTERVAL / portTICK_PERIOD_MS);
    }
    return voltage / counter ;
}

float sen0159GetPercentage(float volts, float *pcurve){
    volts = volts / 100;
    if((volts / DC_GAIN) >= ZERO_POINT_VOLTAGE){
        ESP_LOGE(TAG_SEN0159, "SOME ERROR");
        return -1;
    }else{
        return pow(10, ((volts / DC_GAIN) - pcurve[1]) / pcurve[2] + pcurve[0]);
    }
}