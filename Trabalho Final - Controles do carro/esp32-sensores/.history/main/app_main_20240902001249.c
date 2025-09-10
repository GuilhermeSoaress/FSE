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
#include "driver/ledc.h"
//#define LDR 4
#define ldr_channel ADC_CHANNEL_6

#define LED_1 4 
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
    valor = params_item->valueint;

    printf("Valor: %D\n", valor);
    if (cJSON_IsBool(params_item)) {
        //valor = cJSON_IsTrue(params_item) ? 1 : 0;
        int x =0;
    } else {
        printf("Error getting params\n");
        cJSON_Delete(json);
        return 0;
    }

    // Clean up
    cJSON_Delete(json);
    return valor;
}

void inicializaPwm(){
    // esp_rom_gpio_pad_select_gpio(2);
    // gpio_set_direction(LED_1, GPIO_MODE_OUTPUT);
    // Configuração do Timer
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_config);

    // Configuração do Canal
    ledc_channel_config_t channel_config = {
        .gpio_num = LED_1,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };


    ledc_channel_config(&channel_config);
    ledc_fade_func_install(0);
    
    while(true)
    {
        ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 1000, LEDC_FADE_WAIT_DONE);
        ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 255, 1000, LEDC_FADE_WAIT_DONE);
    }
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
            
            processaMetodo(event->data, method);
                    
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
        .credentials.username = "TQBnkYgCmjOjuav11RZY",
   
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



void dht_test(void *pvParameters)
{
       

#ifdef CONFIG_EXAMPLE_INTERNAL_PULLUP
    gpio_set_pull_mode(dht_gpio, GPIO_PULLUP_ONLY);
#endif
         //-------------ADC1 Init---------------//
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    while (1)
    {
        adc_oneshot_read(adc1_handle, ldr_channel, &adc_raw[0][0]);
        if (adc_raw[0][0] != 0 && adc_raw[0][0] != INVALID_ADC_VALUE) {
            sprintf(luminosidade, "%d",adc_raw[0][0]);
        }else {
            sprintf(luminosidade, "0");
        }
        //printf("LDR: %s\n", luminosidade);
        
        if (dht_read_float_data(DHT_TYPE_DHT11, 5, &humidity, &temperature) == ESP_OK){

            printf("Temperatura: %.2f\n", temperature);
        }
        else
            printf("Could not read data from sensor\n");

        
        sprintf(telemetry, "{\"temperatura\": %.2f, \"umidade\": %.2f, \"luminosidade\" : %s}", temperature, humidity, luminosidade);
        mqtt_envia_mensagem("v1/devices/me/telemetry", telemetry);
      
        sprintf(attributes, "{\"farol\": %d}", farolStatus);
        mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
        vTaskDelay(pdMS_TO_TICKS(3000));
      
        
    }
}

void processaMetodo(const char *data, char *method) {
    int valor = separaParametros(data, method);
    printf("Method: %s\n", method);
    if(strcmp(method, "farol") == 0){
        printf("Status do farol %d\n", farolStatus);
        farolStatus = valor;
        sprintf(attributes, "{\"farol\": %d}", farolStatus);
        mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
    } 
    else if(strcmp(method, "luzInterna")==0){
        printf("Luz Interna %d\n", valor);
        ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, valor, 1000, LEDC_FADE_WAIT_DONE);
    }
    else {
        printf("Método não encontrado\n");
    }
}



void app_main(void)
{
    xTaskCreate(dht_test, "dht_test", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
    xTaskCreate(inicializaPwm, "inicializaPwm", configMINIMAL_STACK_SIZE , NULL, 5, NULL);
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

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();

    //char temperatura[] = temperature;
    //sprintf(convertido, "%.3f", valor);
   
}
