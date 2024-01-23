#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "esp_log.h"

#include <math.h>

#include "mq9.h"

const char *TAG_MQ9 = "MQ9";

static esp_adc_cal_characteristics_t adc1_chars;

/**
 * @brief Calibrate the sensor, this function should be running for several minutes after preheating for 24hours
 * 
 * @param device the mq9 structure data
 */
static void mq9_calibrate(mq9_t *device)
{
    ESP_LOGI(TAG_MQ9, "Calibrating, please wait");
    float calcR0 = 0;
    for (int i = 0; i < 100; i++) {
        uint32_t voltage = adc1_get_raw(device->channel);
        //ESP_LOGI(TAG_MQ9, "ADC VALUE: %d", (int) voltage);
        float voltage_sensor = voltage * VOLTAGE_RANGE / ((float)pow(2, device->width) - 1);
        float resistance_sensor = ((VOLTAGE_RANGE * device->rl) / voltage_sensor) - device->rl;


        //ESP_LOGI(TAG_MQ9, "Voltage: %f, Resistance: %f", voltage_sensor, resistance_sensor);
        calcR0 += resistance_sensor / RATIO_MQ9_CLEAN_AIR;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    device->r0 = calcR0 / 100;
    ESP_LOGI(TAG_MQ9, "Calibration done!");
}

/**
 * @brief The mq9_init function initializes the mq9 sensor
 * 
 * @param device the mq9 structure data
 * @param channel the ADC channel of ESP32
 * @param width the ADC width of ESP32
 * @param atten the attenuation of ADC signal
 * @param a value of a in the equation y = a * x^b
 * @param b value of b in the equation y = a * x^b
 * @param rl load resistance value (potentiometer)
 */
void mq9_init(mq9_t *device, adc_channel_t channel, adc_bits_width_t width, adc_atten_t atten, float a, float b, float rl)
{

    device->channel = channel;
    device->width = width;
    device->atten = atten;
    device->a = a;
    device->b = b;
    device->rl = rl;

    esp_adc_cal_characterize(ADC_UNIT_1, atten, width, DEFAULT_VREF_MQ9, &adc1_chars);
    
    adc1_config_width(width);
    adc1_config_channel_atten(channel, atten);

    mq9_calibrate(device);
}

/**
 * @brief Get the ratio value Rs/R0
 * 
 * @param device the mq9 structure data
 * @return float the ratio value
 */
float mq9_read_ratio(mq9_t *device)
{
    uint32_t voltage = adc1_get_raw(device->channel);
    ESP_LOGI(TAG_MQ9, "ADC VALUE: %d", (int) voltage);
    float voltage_sensor = voltage * VOLTAGE_RANGE / ((float)pow(2, device->width) - 1);
    float resistance_sensor = ((VOLTAGE_RANGE * device->rl) / voltage_sensor) - device->rl;
    float ratio = resistance_sensor / device->r0;

    return ratio;
}

/**
 * @brief Get the ppm value
 * 
 * @param device the mq9 structure data
 * @param ratio the ratio value
 * @return float the ppm value
 */
float mq9_read(mq9_t *device, float ratio)
{
    return device->a * pow(ratio, device->b);
}