/*
 * Driver for Bosch Sensortec BME680 digital temperature, humidity, pressure
 * and gas sensor connected to I2C
 *
 * This driver is for the usage with the ESP8266 and FreeRTOS (esp-open-rtos)
 * [https://github.com/SuperHouse/esp-open-rtos]. It is also working with ESP32
 * and ESP-IDF [https://github.com/espressif/esp-idf.git] as well as Linux
 * based systems using a wrapper library for ESP8266 functions.
 */

#ifndef __BME680_H__
#define __BME680_H__

// Uncomment one of the following defines to enable debug output
// #define BME680_DEBUG_LEVEL_1    // only error messages
// #define BME680_DEBUG_LEVEL_2    // debug and error messages

#include "bme680_types.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "freertos/task.h"
#include <sys/time.h>
#include "driver/uart.h"

/*
 * esp-open-rtos SDK function wrapper
 */

uint32_t sdk_system_get_time();

// BME680 addresses
#define BME680_I2C_ADDRESS_1           0x76  // SDO pin is low
#define BME680_I2C_ADDRESS_2           0x77  // SDO pin is high

// BME680 chip id
#define BME680_CHIP_ID                 0x61    // BME680_REG_ID<7:0>

// Definition of error codes
#define BME680_OK                      0
#define BME680_NOK                     -1

#define BME680_INT_ERROR_MASK          0x000f
#define BME680_DRV_ERROR_MASK          0xfff0

// Error codes for I2C interface ORed with BME680 driver error codes
#define BME680_I2C_READ_FAILED         1
#define BME680_I2C_WRITE_FAILED        2
#define BME680_I2C_BUSY                3

// BME680 driver error codes ORed with error codes for I2C interface
#define BME680_RESET_CMD_FAILED        ( 1 << 8)
#define BME680_WRONG_CHIP_ID           ( 2 << 8)
#define BME680_READ_CALIB_DATA_FAILED  ( 3 << 8)
#define BME680_MEAS_ALREADY_RUNNING    ( 4 << 8)
#define BME680_MEAS_NOT_RUNNING        ( 5 << 8)
#define BME680_MEAS_STILL_RUNNING      ( 6 << 8)
#define BME680_FORCE_MODE_FAILED       ( 7 << 8)
#define BME680_NO_NEW_DATA             ( 8 << 8)
#define BME680_WRONG_HEAT_PROFILE      ( 9 << 8)
#define BME680_MEAS_GAS_NOT_VALID      (10 << 8)
#define BME680_HEATER_NOT_STABLE       (11 << 8)

// Driver range definitions
#define BME680_HEATER_TEMP_MIN         200  // min. 200 degree Celsius
#define BME680_HEATER_TEMP_MAX         400  // max. 200 degree Celsius
#define BME680_HEATER_PROFILES         10   // max. 10 heater profiles 0 ... 9
#define BME680_HEATER_NOT_USED         -1   // heater not used profile

#ifdef __cplusplus
extern "C"
{
#endif

/** --------------------------------------------------------------------------
 *
 * Functional Description of the BME680 sensor
 *
 * The BME680 sensor only support two modes, the sleep mode and the forced
 * mode in which measurements are done. After power-up sequence, the sensor
 * automatically starts in sleep mode. To start a measurement, the sensor has
 * to switch in the forced mode. In this mode it performs exactly one
 * measurement of temperature, pressure, humidity, and gas in that order,
 * the so-called TPHG measurement cycle. After the execution of this TPHG
 * measurement cycle, raw sensor data are available and the sensor returns
 * automatically back to sleep mode.
 *
 * Using the BME680 consists of the following steps
 *
 * 1. Trigger the sensor to switch into forced mode to perform one THPG cycle
 * 2. Wait until the THPG cycle has been finished (measurement duration)
 * 3. Fetch raw sensor data, compensate and convert them to sensor values
 *
 * ---------------------------------------------------------------------------
 */

/**
 * @brief	Initialize a BME680 sensor
 *
 * The function initializes the sensor device data structure, probes the
 * sensor, soft resets the sensor, and configures the sensor with the
 * the following default settings:
 *
 * - Oversampling rate for temperature, pressure, humidity is osr_1x
 * - Filter size for pressure and temperature is iir_size 3
 * - Heater profile 0 with 320 degree C and 150 ms duration
 *
 * @param   addr    I2C addr of the BME680 sensor
 * @return          pointer to sensor data structure, or NULL on error
 */
bme680_sensor_t* bme680_init_sensor (uint8_t addr);

/**
 * @brief	Force one single TPHG measurement
 *
 * The function triggers the sensor to start one THPG measurement cycle.
 * Parameters for the measurement like oversampling rates, IIR filter sizes
 * and heater profile can be configured before.
 *
 * Once the TPHG measurement is started, the user task has to wait for the
 * results. The duration of the TPHG measurement can be determined with
 * function *bme680_get_measurement_duration*.
 *
 * @param   dev   pointer to the sensor device data structure
 * @return        true on success, false on error
 */
bool bme680_force_measurement (bme680_sensor_t* dev);


/**
 * @brief   Get estimated duration of a TPHG measurement
 *
 * The function returns an estimated duration of the TPHG measurement cycle
 * in RTOS ticks for the current configuration of the sensor.
 *
 * This duration is the time required by the sensor for one TPHG measurement
 * until the results are available. It strongly depends on which measurements
 * are performed in the THPG measurement cycle and what configuration
 * parameters were set. It can vary from 1 RTOS (10 ms) tick up to 4500 RTOS
 * ticks (4.5 seconds).
 *
 * If the measurement configuration is not changed, the duration can be
 * considered as constant.
 *
 * @param   dev   pointer to the sensor device data structure
 * @return        duration of TPHG measurement cycle in ticks or 0 on error
 */
uint32_t bme680_get_measurement_duration (const bme680_sensor_t *dev);

/**
 * @brief	Get the measurement status
 *
 * The function can be used to test whether a measurement that was started
 * before is still running.
 *
 * @param   dev   pointer to the sensor device data structure
 * @return        true if measurement is still running or false otherwise
 */
bool bme680_is_measuring (bme680_sensor_t* dev);


/**
 * @brief	Get results of a measurement in fixed point representation
 *
 * The function returns the results of a TPHG measurement that has been
 * started before. If the measurement is still running, the function fails
 * and returns invalid values (see type declaration).
 *
 * @param   dev     pointer to the sensor device data structure
 * @param   results pointer to a data structure that is filled with results
 * @return          true on success, false on error
 */
bool bme680_get_results_fixed (bme680_sensor_t* dev,
                               bme680_values_fixed_t* results);

/**
 * @brief   Get results of a measurement in floating point representation
 *
 * The function returns the results of a TPHG measurement that has been
 * started before. If the measurement is still running, the function fails
 * and returns invalid values (see type declaration).
 *
 * @param   dev     pointer to the sensor device data structure
 * @param   results pointer to a data structure that is filled with results
 * @return          true on success, false on error
 */
bool bme680_get_results_float (bme680_sensor_t* dev,
                               bme680_values_float_t* results);

/**
 * @brief   Start a measurement, wait and return the results (fixed point)
 *
 * This function is a combination of functions above. For convenience it
 * starts a TPHG measurement using *bme680_force_measurement*, then it waits
 * the measurement duration for the results using *vTaskDelay* and finally it
 * returns the results using function *bme680_get_results_fixed*.
 *
 * Note: Since the calling task is delayed using function *vTaskDelay*, this
 * function must not be used when it is called from a software timer callback
 * function.
 *
 * @param   dev     pointer to the sensor device data structure
 * @param   results pointer to a data structure that is filled with results
 * @return          true on success, false on error
 */
bool bme680_measure_fixed (bme680_sensor_t* dev,
                           bme680_values_fixed_t* results);


/**
 * @brief   Start a measurement, wait and return the results (floating point)
 *
 * This function is a combination of functions above. For convenience it
 * starts a TPHG measurement using *bme680_force_measurement*, then it waits
 * the measurement duration for the results using *vTaskDelay* and finally it
 * returns the results using function *bme680_get_results_float*.
 *
 * Note: Since the calling task is delayed using function *vTaskDelay*, this
 * function must not be used when it is called from a software timer callback
 * function.
 *
 * @param   dev     pointer to the sensor device data structure
 * @param   results pointer to a data structure that is filled with results
 * @return          true on success, false on error
 */
bool bme680_measure_float (bme680_sensor_t* dev,
                           bme680_values_float_t* results);

/**
 * @brief   Set the oversampling rates for measurements
 *
 * The BME680 sensor allows to define individual oversampling rates for
 * the measurements of temperature, pressure and humidity. Using an
 * oversampling rate of *osr*, the resolution of raw sensor data can be
 * increased by ld(*osr*) bits.
 *
 * Possible oversampling rates are 1x (default), 2x, 4x, 8x, 16x, see type
 * *bme680_oversampling_rate_t*. The default oversampling rate is 1.
 *
 * Please note: Use *osr_none* to skip the corresponding measurement.
 *
 * @param   dev     pointer to the sensor device data structure
 * @param   ost     oversampling rate for temperature measurements
 * @param   osp     oversampling rate for pressure measurements
 * @param   osh     oversampling rate for humidity measurements
 * @return          true on success, false on error
 */
bool bme680_set_oversampling_rates (bme680_sensor_t* dev,
                                    bme680_oversampling_rate_t osr_t,
                                    bme680_oversampling_rate_t osr_p,
                                    bme680_oversampling_rate_t osr_h);


/**
 * @brief   Set the size of the IIR filter
 *
 * The sensor integrates an internal IIR filter (low pass filter) to reduce
 * short-term changes in sensor output values caused by external disturbances.
 * It effectively reduces the bandwidth of the sensor output values.
 *
 * The filter can optionally be used for pressure and temperature data that
 * are subject to many short-term changes. Using the IIR filter, increases the
 * resolution of pressure and temperature data to 20 bit. Humidity and gas
 * inside the sensor does not fluctuate rapidly and does not require such a
 * low pass filtering.
 *
 * The default filter size is 3 (*iir_size_3*).
 *
 * Please note: If the size of the filter is 0, the filter is not used.
 *
 * @param   dev     pointer to the sensor device data structure
 * @param   size    IIR filter size
 * @return          true on success, false on error
 */
bool bme680_set_filter_size(bme680_sensor_t* dev, bme680_filter_size_t size);


/**
 * @brief   Set a heater profile for gas measurements
 *
 * The sensor integrates a heater for the gas measurement. Parameters for this
 * heater are defined by so called heater profiles. The sensor supports up to
 * 10 heater profiles, which are numbered from 0 to 9. Each profile consists of
 * a temperature set-point (the target temperature) and a heating duration.
 *
 * This function sets the parameters for one of the heater profiles 0 ... 9.
 * To activate the gas measurement with this profile, use function
 * *bme680_use_heater_profile*, see below.
 *
 * Please note: According to the data sheet, a target temperatures of between
 * 200 and 400 degrees Celsius are typical and about 20 to 30 ms are necessary
 * for the heater to reach the desired target temperature.
 *
 * @param   dev           pointer to the sensor device data structure
 * @param   profile       heater profile 0 ... 9
 * @param   temperature   target temperature in degree Celsius
 * @param   duration      heating duration in milliseconds
 * @return                true on success, false on error
 */
bool bme680_set_heater_profile (bme680_sensor_t* dev,
                                uint8_t  profile,
                                uint16_t temperature,
                                uint16_t duration);

/**
 * @brief   Activate gas measurement with a given heater profile
 *
 * The function activates the gas measurement with one of the heater
 * profiles 0 ... 9 or deactivates the gas measurement completely when
 * -1 or BME680_HEATER_NOT_USED is used as heater profile.
 *
 * Parameters of the activated heater profile have to be set before with
 * function *bme680_set_heater_profile* otherwise the function fails.
 *
 * If several heater profiles have been defined with function
 * *bme680_set_heater_profile*, a sequence of gas measurements with different
 * heater parameters can be realized by a sequence of activations of different
 * heater profiles for successive TPHG measurements using this function.
 *
 * @param   dev       pointer to the sensor device data structure0 *
 * @param   profile   0 ... 9 to activate or -1 to deactivate gas measure
 * @return            true on success, false on error
 */
bool bme680_use_heater_profile (bme680_sensor_t* dev, int8_t profile);

/**
 * @brief   Set ambient temperature
 *
 * The heater resistance calculation algorithm takes into account the ambient
 * temperature of the sensor. This function can be used to set this ambient
 * temperature. Either values determined from the sensor itself or from
 * another temperature sensor can be used. The default ambient temperature
 * is 25 degree Celsius.
 *
 * @param   dev           pointer to the sensor device data structure
 * @param   temperature   ambient temperature in degree Celsius
 * @return                true on success, false on error
 */
bool bme680_set_ambient_temperature (bme680_sensor_t* dev,
                                     int16_t temperature);


#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif /* __BME680_H__ */
