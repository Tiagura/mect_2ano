/*
 * ###BME680###
 * GPIO 14 BME680 SCL
 * GPIO 13 BME680 SDA
 * 
 * ###MQ9###
 * ADC1_CHANNEL_5   GPIO 33
 * 
 * ###SEN0159###
 * ADC1_CHANNEL_4   GPIO 32
 * 
 * ###UART_0###
 * BAUD RATE 115200
 * PARITY 8
 * STOP BITS 1
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "driver/uart.h"
#include "driver/i2c.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// user task stack depth for ESP32
#define TASK_STACK_DEPTH 4196

//BME680 includes and defines
#include "bme680.h"
#define I2C_BME_NUM       I2C_NUM_0
#define I2C_BME_SCL_PIN   14
#define I2C_BME_SDA_PIN   13
#define I2C_BME_FREQ      400000

static bme680_sensor_t* sensor = 0;

//MQ9 includes and defines
#include "mq9.h"
#define A                   1000.5
#define B                   -2.186
#define RL                  2500.0

float mq9_value;
mq9_t mq9;

//SEN0159 includes and defines
#include "sen0159.h"

sen0159_t sen0159;


// API includes and defines
#include "api.h"    //for sending data to API
#include "uart.h"   //for getting credentials for Wi-Fi

char ssid[32];
char password[64];

static const char *TAG = "SENDER";

/*
 * User task that triggers measurements of sensor every seconds. It uses
 * function *vTaskDelay* to wait for measurement results.
 * BME680
 * 1s
 */
void bme680_work(void *pvParameters)
{
    bme680_values_float_t values;

    TickType_t last_wakeup = xTaskGetTickCount();

    // as long as sensor configuration isn't changed, duration is constant
    uint32_t duration = bme680_get_measurement_duration(sensor);


    while (1)
    {
        // trigger the sensor to start one TPHG measurement cycle
        if (bme680_force_measurement (sensor))
        {
            // passive waiting until measurement results are available
            vTaskDelay(duration / portTICK_PERIOD_MS);

            // alternatively: busy waiting until measurement results are available
            while (bme680_is_measuring (sensor));

            // get the results and do something with them
            if (bme680_get_results_float (sensor, &values)){
                ESP_LOGI(TAG, "BME680 Sensor: %.2f °C, %.2f %%, %.2f hPa, %.2f IAQ",
                       values.temperature, values.humidity,
                       values.pressure, values.gas_resistance);

                api_add_bme680(values.temperature, values.humidity, values.pressure, values.gas_resistance);
            }else
                printf("error reading values\n");
        }
        // passive waiting until 5 second is over
        vTaskDelayUntil(&last_wakeup, 5000 / portTICK_PERIOD_MS);
    }
}

/*
 * Function to configure communication between ESP32 and BME680
 * I2C, etc...
 */
void bme680_init(){
    // Set UART Parameter.
    //uart_set_baudrate(0, 115200);
    // Give the UART some time to settle
    vTaskDelay(1);
    // Configure I2C communication
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_BME_SDA_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_BME_SCL_PIN;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_BME_FREQ;
    conf.clk_flags = 0;
    i2c_param_config(I2C_BME_NUM, &conf);
    i2c_driver_install(I2C_BME_NUM, conf.mode, 0, 0, 0);

    // Init the sensor with slave address BME680_I2C_ADDRESS_1 connected to I2C_NUM.
    sensor = bme680_init_sensor (BME680_I2C_ADDRESS_1);
    if(sensor){
        ESP_LOGI(TAG, "BME680 initialized successfully");
    }
    else{
        ESP_LOGE(TAG, "BME680 initialization failed");
    }
}

/*
 * Function to configure bme680 measurement params
 */
void bme680_config_msr(){
    if(sensor){
        /** -- SENSOR CONFIGURATION PART (optional) --- */

        // Changes the oversampling rates to 4x oversampling for temperature
        // and 2x oversampling for humidity and pressure.
        bme680_set_oversampling_rates(sensor, osr_4x, osr_2x, osr_2x);

        // Change the IIR filter size for temperature and pressure to 7.
        bme680_set_filter_size(sensor, iir_size_7);

        // Change the heater profile 0 to 200 degree Celcius for 100 ms.
        bme680_set_heater_profile (sensor, 0, 200, 100);
        bme680_use_heater_profile (sensor, 0);

        // Set ambient temperature to 10 degree Celsius
        bme680_set_ambient_temperature (sensor, 20);
    }
    else{
        ESP_LOGE(TAG, "BME680 not initialized");
    }
}

/*
 * User task that triggers measurements of sensor every seconds. It uses
 * function *vTaskDelay* to wait for measurement results.
 * MQ9
 * 1s
 */
void mq9_work(){
    while(1){
        mq9_value = mq9_read(&mq9, mq9_read_ratio(&mq9));
        ESP_LOGI(TAG, "MQ-9 Sensor: %.2f PPM", mq9_value);
        api_add_co(mq9_value);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

/*
 * User task that triggers measurements of sensor every 5 seconds. 
 */
void sen0159_work(){
    while(1){
        float voltage = sen0159Read(&sen0159);
        ESP_LOGI(TAG, "SEN0159 -> Voltage: %f", voltage);
        
        float percentage = sen0159GetPercentage(voltage, CO2Curve);
        if(percentage == -1){
            ESP_LOGE(TAG, "SEN0159 -> Reading: <400ppm");
        }else{
            ESP_LOGI(TAG, "SEN0159 -> Percentage: %f", percentage);
            api_add_co2(percentage);
        }
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

/*
 * Function to get SSID and password from terminal
 */
void getCredentials(){
    printf("Enter SSID: ");
    get_string(ssid, true);
    //strcpy(ssid, "idk");
    printf("Enter password: ");
    get_string(password, false);
    //strcpy(password, "tiagomarques");
    printf("SSID: %s\n", ssid);
    printf("Password: %s\n", password);
}

void app_main(void)
{

    //init uart to get credentials from terminal
    uart_init();

    //to get credentials from terminal
    getCredentials();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //connect to wifi
    connect_wifi(ssid, password);

    //wait 10 seconds for wifi to connect
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    //init I2C interface for BME680
    bme680_init();
    //config bme680 measurement params
    bme680_config_msr();
    xTaskCreate(bme680_work, "bme680_work", TASK_STACK_DEPTH, NULL, 2, NULL);
    ESP_LOGI(TAG, "BME680 configured and task created");

    //init ADC interface for MQ9
    mq9_init(&mq9, ADC_CHANNEL_5, ADC_WIDTH_BIT_12, ADC_ATTEN_DB_11, A, B, RL);
    ESP_LOGI(TAG, "MQ-9 sensor initialized successfully");
    xTaskCreate(mq9_work, "mq9_work", TASK_STACK_DEPTH, NULL, 2, NULL);
    ESP_LOGI(TAG, "MQ-9 task created successfully");

    //init ADC interface for SEN0159
    sen0159Init(&sen0159, MG_PIN, ADC_WIDTH_BIT_12, ADC_ATTEN_DB_11);
    ESP_LOGI(TAG, "SEN0159 initialized successfully");
    xTaskCreate(sen0159_work, "sen0159_work", TASK_STACK_DEPTH, NULL, 2, NULL);
    ESP_LOGI(TAG, "SEN0159 task created successfully");
}
