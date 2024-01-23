#pragma once
#include <stdint.h>

#include <esp_http_client.h>
#include <esp_log.h>
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"


#define API_URL "http://192.168.20.217:8000"

#define WIFI_FAIL_BIT BIT1
#define WIFI_CONNECTED_BIT BIT0
#define EXAMPLE_ESP_MAXIMUM_RETRY 10

/**
 * sen0159 
 * @param co2
 * POST: sen0159/
*/
void api_add_co2(float co2);

/**
 * mq9 
 * @param co
 * POST: mq9/
*/
void api_add_co(float co);

/**
 * bme680 
 * @param temperature
 * @param humidity
 * @param pressure
 * @param gas
 * POST: bme680/
*/
void api_add_bme680(float temperature, float humidity, float pressure, float gas);


/**
 * Used to connect to wifi
 * @param ssid
 * @param password
*/
void connect_wifi(char *ssid, char *password);

void event_handler(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data);

