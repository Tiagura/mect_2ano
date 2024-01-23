#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <esp_http_client.h>
#include <esp_log.h>
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"

#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>

#include "cJSON.h"

#include "api.h"

static const char *TAG = "API";

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

void api_add_co2(float co2) {
    esp_http_client_config_t config = {
        .url = API_URL "/sen0159/",
        .cert_pem = NULL,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    // Set the HTTP method to POSTs
    esp_http_client_set_method(client, HTTP_METHOD_POST);

    //create json
    cJSON *json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "co2", co2);
    //convert json to string
    char *json_string = cJSON_Print(json);

    if(json_string == NULL){
        ESP_LOGE(TAG, "Failed to print json string");
        cJSON_Delete(json);
        return;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_string, strlen(json_string));

    // Perform the HTTP POST request
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        //log status response and the url
        ESP_LOGI(TAG, "%s\nHTTP POST Status = %d",
                API_URL "/sen0159/",
                esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "%s HTTP POST request failed: %s", API_URL "/sen0159/", esp_err_to_name(err));
    }

    // Delete JSON
    cJSON_Delete(json);
    
    // Cleanup
    esp_http_client_cleanup(client);
}



void api_add_co(float co){
    esp_http_client_config_t config = {
        .url = API_URL "/mq9/",
        .cert_pem = NULL,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    // Set the HTTP method to POSTs
    esp_http_client_set_method(client, HTTP_METHOD_POST);

    //create json
    cJSON *json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "co", co);
    //convert json to string
    char *json_string = cJSON_Print(json);

    if(json_string == NULL){
        ESP_LOGE(TAG, "Failed to print json string");
        cJSON_Delete(json);
        return;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_string, strlen(json_string));

    // Perform the HTTP POST request
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        //log status response and the url
        ESP_LOGI(TAG, "%s\nHTTP POST Status = %d",
                API_URL "/mq9/",
                esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "%s HTTP POST request failed: %s", API_URL "/mq9/", esp_err_to_name(err));
    }

    // Delete JSON
    cJSON_Delete(json);
    
    // Cleanup
    esp_http_client_cleanup(client);
}

void api_add_bme680(float temperature, float humidity, float pressure, float gas){
    esp_http_client_config_t config = {
        .url = API_URL "/bme680/",
        .cert_pem = NULL,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    // Set the HTTP method to POSTs
    esp_http_client_set_method(client, HTTP_METHOD_POST);

    //create json
    cJSON *json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "temperature", temperature);
    cJSON_AddNumberToObject(json, "humidity", humidity);
    cJSON_AddNumberToObject(json, "pressure", pressure);
    cJSON_AddNumberToObject(json, "gas", gas);
    //convert json to string
    char *json_string = cJSON_Print(json);

    if(json_string == NULL){
        ESP_LOGE(TAG, "Failed to print json string");
        cJSON_Delete(json);
        return;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_string, strlen(json_string));

    // Perform the HTTP POST request
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        //log status response and the url
        ESP_LOGI(TAG, "%s\nHTTP POST Status = %d",
                API_URL "/bme680/",
                esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "%s HTTP POST request failed: %s", API_URL "/bme680/", esp_err_to_name(err));
    }

    // Delete JSON
    cJSON_Delete(json);
    
    // Cleanup
    esp_http_client_cleanup(client);

}

void connect_wifi(char *ssid, char *password)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&event_handler,NULL,&instance_any_id));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&event_handler,NULL,&instance_got_ip));


    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, password);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");


    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,pdFALSE,pdFALSE,portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "Connected to SSID");
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect");
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    vEventGroupDelete(s_wifi_event_group);
}

void event_handler(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            // force restart
            esp_restart();
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);

            
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "GOT IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}