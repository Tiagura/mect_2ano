#ifndef SEN0159_H
#define SEN0159_H

#include "driver/adc.h"
#include "esp_adc_cal.h"

//#define ZERO_POINT_VOLTAGE (voltage/8.5) //mesure the output voltage of this module after 48h calibration

//HARDWARE MACROS
#define MG_PIN ADC1_CHANNEL_4       //define which analog input channel you are going to use, ADC1_CHANNEL_4 is GPIO32
#define DEFAULT_VREF_SEN0159 3.3            //ESP32 ADC's reference voltage
#define DC_GAIN 8.5                 //define the DC gain of amplifier


//SOFTWARE MACROS
#define NUMBER_SAMPLES 50           //define how many samples you are going to take in normal operation
#define READ_SAMPLE_INTERVAL 5      //define the time interval(in milisecond) between each samples in normal operation

//APP MACROS
#define ZERO_POINT_VOLTAGE 0.220    //define the output of the sensor in volts when the concentration of CO2 is 400PPM
#define REACTION_VOLTAGE 0.030      //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2

//GLOBAL VARIABLES
extern float CO2Curve[3];

//STRUCTURES
typedef struct {
    adc1_channel_t channel;
    adc_bits_width_t width;
    adc_atten_t atten;
    esp_adc_cal_characteristics_t adc_chars;
} sen0159_t;

//FUNCTIONS
/*
* Input: device - pointer to the sen0159_t structure
*        channel - ADC1 channel of ESP32
*        width - ADC width of ESP32
*        atten - attenuation of ADC signal
* Output: none
*/
void sen0159Init(sen0159_t *device, adc_channel_t channel, adc_bits_width_t width, adc_atten_t atten);


/*
* Input: mg_pin - ADC pin number
* Output: voltage - voltage read from ADC
* Description: read the voltage from the ADC pin
*/
float sen0159Read(sen0159_t *device);

/*
* Input: volts - voltage read from ADC
*        pcurve - pointer to the curve of the sensor
* Output: ppm of the CO2
* Description: calculate the ppm of CO2. 
* Remakrs: By using the slope and a point of the line. The x(logarithmic value of ppm)
            of the line could be derived if y(MG-811 output) is provided. As it is a
            logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic
            value.
*/
float sen0159GetPercentage(float volts, float *pcurve);

#endif