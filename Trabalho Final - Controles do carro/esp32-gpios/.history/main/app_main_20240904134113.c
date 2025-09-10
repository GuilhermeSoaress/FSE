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
#include "wifi_manager.h"
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
int portaStatus = 0;
int posChaveStatus = 0;
int freioStatus = 0;
int botaoTravarStatus = 0;
int travadoStatus = 0;
int i =1;

//Pinos
#define porta 16
#define posChave 17
#define freio 5
#define botaoTravar 4

QueueHandle_t filaDeInterrupcao;

void configuraPinos(){
    esp_rom_gpio_pad_select_gpio(porta);
    gpio_set_direction(porta, GPIO_MODE_INPUT);
    gpio_pulldown_en(porta);
    gpio_pullup_dis(porta);

    esp_rom_gpio_pad_select_gpio(posChave);
    gpio_set_direction(posChave, GPIO_MODE_INPUT);
    gpio_pulldown_en(posChave);
    gpio_pullup_dis(posChave);
    
    esp_rom_gpio_pad_select_gpio(freio);
    gpio_set_direction(freio, GPIO_MODE_INPUT);
    gpio_pulldown_en(freio);
    gpio_pullup_dis(freio);

    esp_rom_gpio_pad_select_gpio(botaoTravar);
    gpio_set_direction(botaoTravar, GPIO_MODE_INPUT);
    gpio_pulldown_en(botaoTravar);
    gpio_pullup_dis(botaoTravar);

    gpio_set_intr_type(porta, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(posChave, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(freio, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(botaoTravar, GPIO_INTR_ANYEDGE);

}
void mqtt_envia_mensagem(char * topico, char * mensagem)
{
    int message_id = esp_mqtt_client_publish(client, topico, mensagem, 0, 1,0 );
    ESP_LOGI(TAG, "Mensagem enviada, ID: %d", message_id);
}
void mqtt_envia_mensagem2(char * topico, char * mensagem)
{
    int message_id = esp_mqtt_client_publish(client2, topico, mensagem, 0, 1,0 );
    ESP_LOGI(TAG, "Mensagem enviada, ID: %d", message_id);
}
static void IRAM_ATTR gpio_isr_handler(void *args)
{
  int pino = (int)args;
  xQueueSendFromISR(filaDeInterrupcao, &pino, NULL);
}

void trataInterrupcaoBotao(void *params)
{
  int pino;
  //int contador = 0;

  while(true)
  {
    if(xQueueReceive(filaDeInterrupcao, &pino, portMAX_DELAY))
    {
      // De-bouncing
      int estado = gpio_get_level(pino);
    //   if(estado == 1)
    //   {
        gpio_isr_handler_remove(pino);
        printf("Botão %d pressionado\n", pino);
        switch (pino)
        {
        case porta:
            portaStatus = estado; 
            snprintf(attributes, sizeof(attributes), "{\"porta\": %d, \"posChave\": %d, \"freio\": %d, \"botaoTravar\": %d, \"travadoStatus\": %d}", portaStatus, posChaveStatus, freioStatus, botaoTravarStatus);
            mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
            mqtt_envia_mensagem2("v1/devices/me/attributes/gpios", attributes);
             //vTaskDelay(500/portTICK_PERIOD_MS);
            break;
        case posChave:
            posChaveStatus = estado;
            snprintf(attributes, sizeof(attributes), "{\"porta\": %d, \"posChave\": %d, \"freio\": %d, \"botaoTravar\": %d, \"travadoStatus\": %d}", portaStatus, posChaveStatus, freioStatus, botaoTravarStatus, travadoStatus);
            mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
            mqtt_envia_mensagem2("v1/devices/me/attributes/gpios", attributes);
             //vTaskDelay(500/portTICK_PERIOD_MS);
            break;
        case freio:
            freioStatus = estado;
            snprintf(attributes, sizeof(attributes), "{\"porta\": %d, \"posChave\": %d, \"freio\": %d, \"botaoTravar\": %d, \"travadoStatus\": %d}", portaStatus, posChaveStatus, freioStatus, botaoTravarStatus, travadoStatus);
            mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
            mqtt_envia_mensagem2("v1/devices/me/attributes/gpios", attributes);
            break;
            //vTaskDelay(500/portTICK_PERIOD_MS);
        case botaoTravar:
            botaoTravarStatus = estado;
            snprintf(attributes, sizeof(attributes), "{\"porta\": %d, \"posChave\": %d, \"freio\": %d, \"botaoTravar\": %d, \"travadoStatus\": %d}", portaStatus, posChaveStatus, freioStatus, botaoTravarStatus, travadoStatus);
            mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
            mqtt_envia_mensagem2("v1/devices/me/attributes/gpios", attributes);
            (botaoTravarStatus) ? travar(), travadoStatus = 1 : destravar(), travadoStatus = 0;
            vTaskDelay(1000/portTICK_PERIOD_MS);
            estado = gpio_get_level(pino);
            botaoTravarStatus = estado;
            snprintf(attributes, sizeof(attributes), "{\"porta\": %d, \"posChave\": %d, \"freio\": %d, \"botaoTravar\": %d, \"travadoStatus\": %d}", portaStatus, posChaveStatus, freioStatus, botaoTravarStatus, travadoStatus);
            mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
            mqtt_envia_mensagem2("v1/devices/me/attributes/gpios", attributes);

            
            break;

        default:
            break;
        }
        // while(gpio_get_level(pino) == estado)
        // {
        //   vTaskDelay(50 / portTICK_PERIOD_MS);
        // }

        // contador++;
        // printf("Os botões foram acionados %d vezes. Botão: %d\n", contador, pino);
        // Habilitar novamente a interrupção
        vTaskDelay(50 / portTICK_PERIOD_MS);
        gpio_isr_handler_add(pino, gpio_isr_handler, (void *) pino);
      

    }
  }
}


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
 

// void processaMetodo(const char *data, char *method) {
//     int valor = separaParametros(data, method);
//     printf("Method: %s\n", method);
//     if(strcmp(method, "farol") == 0){
//         farois(valor);
//         printf("Status do farol %d\n", farolStatus);
//         farolStatus = valor;
//         sprintf(attributes, "{\"farol\": %d}", farolStatus);
//         mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
//     } 
//     else if(strcmp(method, "luzInterna")==0){
//         printf("Luz Interna %d\n", valor);
//         luzInternaPwm = valor;
//         int duty = (int)(valor * 255 / 100);
//         ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
//         ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
//         sprintf(attributes, "{\"luzInterna\": %d}", luzInternaPwm);
//         mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
//     }
//     else {
//         printf("Método não encontrado\n");
//     }
// }
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
            
            msg_id = esp_mqtt_client_subscribe(client2, "v1/devices/me/telemetry/+", 0);
            esp_mqtt_client_subscribe(client2, "v1/devices/me/attributes/sensores" , 0);
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
            
            //processaMetodo(event->data, method);
                    
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
// void enviaEstadoGpio(){
//     while(1){
//         portaStatus = gpio_get_level(porta);
//         posChaveStatus = gpio_get_level(posChave);
//         freioStatus = gpio_get_level(freio);
//         botaoTravarStatus = gpio_get_level(botaoTravar);
//         snprintf(attributes, sizeof(attributes), "{\"porta\": %d, \"posChave\": %d, \"freio\": %d, \"botaoTravar\": %d}", portaStatus, posChaveStatus, freioStatus, botaoTravarStatus);
//         mqtt_envia_mensagem("v1/devices/me/attributes/", attributes);
//         mqtt_envia_mensagem2("v1/devices/me/attributes/gpios", attributes);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }
void monitoring_task(void *pvParameter)
{
    for(;;){
        // Modificação: Use PRIu32 para imprimir o tamanho da heap corretamente
        ESP_LOGI(TAG, "free heap: %" PRIu32, esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/**
 * @brief this is an example of a callback that you can setup in your own app to get notified of wifi manager event.
 */
void cb_connection_ok(void *pvParameter){
    ip_event_got_ip_t* param = (ip_event_got_ip_t*)pvParameter;

    /* transform IP to human readable string */
    char str_ip[16];
    esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);

    ESP_LOGI(TAG, "I have a connection and my IP is %s!", str_ip);
}

void app_main(void)
{
    /* Initialize NVS */
    ESP_ERROR_CHECK(nvs_flash_init());

    /* Start the WiFi Manager */
    wifi_manager_start();

    /* Register a callback to handle the event when the WiFi gets an IP */
    wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &cb_connection_ok);

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

    mqtt_app_start();
    configuraPinos();
    
    //xTaskCreate(enviaEstadoGpio, "enviaEstadoGpio", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);

    filaDeInterrupcao = xQueueCreate(10, sizeof(int));
    xTaskCreate(trataInterrupcaoBotao, "TrataBotao", 2048, NULL, 1, NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(porta, gpio_isr_handler, (void *) porta);
    gpio_isr_handler_add(posChave, gpio_isr_handler, (void *) posChave);
    gpio_isr_handler_add(freio, gpio_isr_handler, (void *) freio);
    gpio_isr_handler_add(botaoTravar, gpio_isr_handler, (void *) botaoTravar);

}
