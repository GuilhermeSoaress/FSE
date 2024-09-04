/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
//#include <json-c/json.h>
#include <cJSON.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "dht.h"

#include "esp_adc/adc_oneshot.h"
#include "controle.h"
//#define LDR 4
#define ldr_channel ADC_CHANNEL_6
//adc2_channel 0
// Configura o conversor AD
#define INVALID_ADC_VALUE -1
static int adc_raw[2][10];
        

static const char *TAG = "mqtt_example";

esp_mqtt_client_handle_t client;
esp_mqtt_client_handle_t client2;

float temperature, humidity;
// char temperatura[16] ;
// char umidade[16];
char luminosidade[16];
int requestID=0;
int requestID2=0;
int farolStatus = 0;
char attributes[100];
char telemetry[100]; 
char method[50];

int i =1;

int separaParametros(const char *json_str, char *method) {
    
    int valor;
    // Parse the JSON string
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
        printf("Error parsing JSON\n");
        return 0;
    }

    // Get "method"
    cJSON *method_item = cJSON_GetObjectItem(json, "method");
    if (cJSON_IsString(method_item) && (method_item->valuestring != NULL)) {
        snprintf(method, 256, "%s", method_item->valuestring);
    } else {
        printf("Error getting method\n");
        cJSON_Delete(json);
        return 0;
    }

    // Get "params"
    cJSON *params_item = cJSON_GetObjectItem(json, "params");
    if (cJSON_IsBool(params_item)) {
        valor = cJSON_IsTrue(params_item) ? 1 : 0;
    } else {
        printf("Error getting params\n");
        cJSON_Delete(json);
        return 0;
    }

    // Clean up
    cJSON_Delete(json);
    return valor;
}
 
void mqtt_envia_mensagem(char * topico, char * mensagem)
{
    int message_id = esp_mqtt_client_publish(client, topico, mensagem, 0, 1,0 );
    int message_id2 = esp_mqtt_client_publish(client2, topico, mensagem, 0, 1,0 );
    ESP_LOGI(TAG, "Mensagem enviada, ID: %d", message_id);
    ESP_LOGI(TAG, "Mensagem enviada, ID: %d", message_id2);
}
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    
    esp_mqtt_event_handle_t event2 = event_data;
    esp_mqtt_client_handle_t client2 = event2->client;
    int msg_id;
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "v1/devices/me/rpc/request/+", 0);
            esp_mqtt_client_subscribe(client, "v1/devices/me/attributes/response/+" , 0);
            
            msg_id = esp_mqtt_client_subscribe(client2, "v1/devices/me/rpc/request/+", 0);
            esp_mqtt_client_subscribe(client2, "v1/devices/me/attributes/response/+" , 0);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            requestID = event->event_id;
            requestID2 = event2->event_id;
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("Event ID: %d\n", requestID);
            printf("DATA=%.*s\r\n", event->data_len, event->data);

            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("Event ID: %d\n", requestID2);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            
            // int valor = separaParametros(event->data, method);
            // printf("Method: %s\n", method);
            // if(strcmp(method, "farol") == 0){
            //     farois(valor);
            //     printf("Status do farol %d\n", farolStatus);
            //     farolStatus = valor;
            //     sprintf(attributes, "{\"farol\": %d}", farolStatus);
            //     mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
            } 
            else printf("Método não encontrado\n"); 
                    
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }

}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://164.41.98.25",
        .credentials.username = "NXx0lCh9DAaV2o4yDjYm",
   
    };
    esp_mqtt_client_config_t mqtt_cfg2 = {
        .broker.address.uri = "mqtt://192.168.1.27"
    };
    #if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.broker.address.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.broker.address.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif
    
    client = esp_mqtt_client_init(&mqtt_cfg);
    client2 = esp_mqtt_client_init(&mqtt_cfg2);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL );
    esp_mqtt_client_register_event(client2, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL );
    esp_mqtt_client_start(client);
    esp_mqtt_client_start(client2);
}




void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);
    esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("transport", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();
  
}
