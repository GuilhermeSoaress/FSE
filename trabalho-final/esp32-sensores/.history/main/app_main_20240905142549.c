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
#include "esp_sleep.h"
#include "esp32/rom/uart.h"
#include "driver/rtc_io.h"
#include "wifi_manager.h"

#define BOTAO 4
#define ldr_channel ADC_CHANNEL_6
// #define LED_1 15 
#define dhtPin 25

//INPUTS
#define botaoFarol 17
#define botaoVidros 5
#define botaoTrava 18

// Configura o conversor AD
#define INVALID_ADC_VALUE -1
static int adc_raw[2][10];

static const char *TAG = "Relampago Marquinhos";

esp_mqtt_client_handle_t client;
esp_mqtt_client_handle_t client2;

float temperature, humidity;
char temperatura[10] ;
char umidade[10];
char luminosidade[10];
char botaoFarolStr[10];
char luzInternaStr[10];
char botaoVidrosStr[10];
char botaoTravaStr[10];
int requestID=0;
int requestID2=0;

int farolStatus = 0;
int botaoVidrosStatus = 0;
int botaoTravarStatus = 0;
int travadoStatus = 0;

char attributes[100];
char telemetry[100]; 
char method[50];


int i =1;

QueueHandle_t filaDeInterrupcao;

void configuraPinos(){
    esp_rom_gpio_pad_select_gpio(botaoFarol);
    gpio_set_direction(botaoFarol, GPIO_MODE_INPUT);
    gpio_pulldown_en(botaoFarol);
    gpio_pullup_dis(botaoFarol);

    esp_rom_gpio_pad_select_gpio(botaoVidros);
    gpio_set_direction(botaoVidros, GPIO_MODE_INPUT);
    gpio_pulldown_en(botaoVidros);
    gpio_pullup_dis(botaoVidros);

    esp_rom_gpio_pad_select_gpio(botaoTrava);
    gpio_set_direction(botaoTrava, GPIO_MODE_INPUT);
    gpio_pulldown_en(botaoTrava);
    gpio_pullup_dis(botaoTrava);
    
    gpio_set_intr_type(botaoFarol, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(botaoVidros, GPIO_INTR_HIGH_LEVEL);
    gpio_set_intr_type(botaoTrava, GPIO_INTR_HIGH_LEVEL);
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
   
        gpio_isr_handler_remove(pino);
        printf("Botão %d pressionado\n", pino);
        switch (pino)
        {
        case botaoFarol:
            
            farolStatus = estado; 
            snprintf(attributes, sizeof(attributes), "{\"farol\": %d, \"botaoVidros\": %d, \"botaoTravar\": %d, \"travadoStatus\": %d}", farolStatus,botaoVidrosStatus, botaoTravarStatus, travadoStatus);
            mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
            
            sprintf(botaoFarolStr, "%d", farolStatus);
            mqtt_envia_mensagem2("sensores/farol" , botaoFarolStr);
            break;
        
        case botaoVidros:
            
            snprintf(attributes, sizeof(attributes), "{\"farol\": %d, \"botaoVidros\": %d, \"botaoTravar\": %d, \"travadoStatus\": %d}", farolStatus,1, botaoTravarStatus, travadoStatus);
            mqtt_envia_mensagem("v1/devices/me/attributes", attributes);

            sprintf(botaoVidrosStr, "%d", 1);
            mqtt_envia_mensagem2("sensores/vidros", botaoVidrosStr);
            
            break;
        
        case botaoTrava:
            
            travadoStatus = !travadoStatus;

            snprintf(attributes, sizeof(attributes), "{\"farol\": %d, \"botaoVidros\": %d, \"travadoStatus\": %d}", farolStatus, botaoVidrosStatus, travadoStatus);
            mqtt_envia_mensagem("v1/devices/me/attributes", attributes);

            sprintf(botaoTravaStr, "%d", travadoStatus);
            mqtt_envia_mensagem2("sensores/travadoStatus", botaoTravaStr);
            break;

        default:
            break;
        }
        
        vTaskDelay(50 / portTICK_PERIOD_MS);
        gpio_isr_handler_add(pino, gpio_isr_handler, (void *) pino);

    }
  }
} 
int separaParametros(const char *json_str, char *method) {
    
    int valor;
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
        printf("Error parsing JSON\n");
        return 0;
    }

    cJSON *method_item = cJSON_GetObjectItem(json, "method");
    if (cJSON_IsString(method_item) && (method_item->valuestring != NULL)) {
        snprintf(method, 256, "%s", method_item->valuestring);
    } else {
        printf("Error getting method\n");
        cJSON_Delete(json);
        return 0;
    }

    cJSON *params_item = cJSON_GetObjectItem(json, "params");
    valor = params_item->valueint;

    cJSON_Delete(json);
    return valor;
}

 void processaMetodo(const char *data, char *method) {    
     int valor = separaParametros(data, method);
     printf("Method: %s\n", method);
     if(strcmp(method, "travadoStatus")==0){
        travadoStatus = valor;
        sprintf(attributes, "{\"farol\": %d, \"botaoVidros\": %d, \"botaoTravar\": %d, \"travadoStatus\": %d}", farolStatus, botaoVidrosStatus, botaoTravarStatus, travadoStatus);
        mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
        char status[10];
        sprintf(status, "%d", travadoStatus);
        mqtt_envia_mensagem2("sensores/travadoStatus", status);
     }
    else if(strcmp(method, "farol") == 0){
        //farois(valor);
        printf("Status do farol %d\n", farolStatus);
        farolStatus = valor;
        sprintf(attributes, "{\"farol\": %d}", farolStatus);
        mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
        char statusFarol[10];
        sprintf(statusFarol, "%d", farolStatus);
        mqtt_envia_mensagem2("sensores/farol", statusFarol);
    }
    else if(strcmp(method, "vidros")){
         
        int vidroStatus = 1;
        sprintf(attributes, "{\"vidros\": %d}", vidroStatus);
        mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
        char statusVidros[10];
        sprintf(statusVidros, "%d", 1);
        mqtt_envia_mensagem2("sensores/vidros", statusVidros);
        
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        vidroStatus = 0;
        sprintf(attributes, "{\"vidros\": %d}", vidroStatus);
        mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
        sprintf(statusVidros, "%d", vidroStatus);
        mqtt_envia_mensagem2("sensores/vidros", statusVidros);
    }
    else if(strcmp(method, "luzInterna")==0){
        printf("Luz Interna %d\n", valor);
        luzInternaPwm = valor;
        int duty = (int)(valor * 255 / 100);
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        sprintf(attributes, "{\"luzInterna\": %d}", luzInternaPwm);
        mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
    }
    else {
        printf("Método não encontrado\n");
    }
}
// void inicializaPwm(){
//     ledc_timer_config_t timer_config = {
//         .speed_mode = LEDC_HIGH_SPEED_MODE,
//         .duty_resolution = LEDC_TIMER_8_BIT,
//         .timer_num = LEDC_TIMER_0,
//         .freq_hz = 1000,
//         .clk_cfg = LEDC_AUTO_CLK
//     };
//     esp_err_t timer_status = ledc_timer_config(&timer_config);
//     if (timer_status != ESP_OK) {
//         ESP_LOGE(TAG, "Erro ao configurar o timer: %s", esp_err_to_name(timer_status));
//         return;
//     }
//     ESP_LOGI(TAG, "Timer configurado com sucesso.");
//     ledc_channel_config_t channel_config = {
//         .gpio_num = LED_1,
//         .speed_mode = LEDC_HIGH_SPEED_MODE,
//         .channel = LEDC_CHANNEL_0,
//         .timer_sel = LEDC_TIMER_0,
//         .duty = 0,
//         .hpoint = 0
//     };
//     esp_err_t channel_status = ledc_channel_config(&channel_config);
//     if (channel_status != ESP_OK) {
//         ESP_LOGE(TAG, "Erro ao configurar o canal: %s", esp_err_to_name(channel_status));
//         return;
//     }
//     ESP_LOGI(TAG, "Canal configurado com sucesso.");
//}
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    
    esp_mqtt_event_handle_t event2 = event_data;
    esp_mqtt_client_handle_t client2 = event2->client;
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            esp_mqtt_client_subscribe(client, "v1/devices/me/rpc/request/+", 0);
            esp_mqtt_client_subscribe(client, "v1/devices/me/attributes/response/+" , 0);
            
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
        //.broker.address.uri = "mqtt://192.168.168.82"
        .broker.address.uri = "mqtt://192.168.134.82"
    };
    
    client = esp_mqtt_client_init(&mqtt_cfg);
    client2 = esp_mqtt_client_init(&mqtt_cfg2);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL );
    esp_mqtt_client_register_event(client2, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL );
    esp_mqtt_client_start(client);
    esp_mqtt_client_start(client2);
}
void leituraSensores(void *pvParameters)
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
        
        if (dht_read_float_data(DHT_TYPE_DHT11, dhtPin, &humidity, &temperature) == ESP_OK){

            printf("Temperatura: %.2f\n", temperature);
        }
        else
            printf("Could not read data from sensor\n");
        
        sprintf(telemetry, "{\"temperatura\": %.2f, \"umidade\": %.2f, \"luminosidade\" : %s}", temperature, humidity, luminosidade);
        mqtt_envia_mensagem("v1/devices/me/telemetry", telemetry);
        //sprintf(attributes, "{\"farol\": %d, \"luzInterna\": %d}", farolStatus, luzInternaPwm);
        //mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
        
        //Envia mensagens mqtt para o broker local
        sprintf(temperatura, "%.2f", temperature);
        sprintf(umidade, "%.2f", humidity);
        
        mqtt_envia_mensagem2("sensores/temperatura", temperatura);
        mqtt_envia_mensagem2("sensores/umidade", umidade);
        mqtt_envia_mensagem2("sensores/luminosidade", luminosidade); 
        vTaskDelay(pdMS_TO_TICKS(3000));
      
        
    }
}
void configuraSleep()
{
    // Configuração da GPIO para o botão de entrada
    esp_rom_gpio_pad_select_gpio(BOTAO);
    gpio_set_direction(BOTAO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BOTAO, GPIO_PULLUP_ONLY);
    // Habilita o botão para acordar a placa
    gpio_wakeup_enable(BOTAO, GPIO_INTR_HIGH_LEVEL);
    
    esp_sleep_enable_gpio_wakeup();

  while(true)
  {

    if (rtc_gpio_get_level(BOTAO) == 1)
    {
        printf("Acordando... \n");
        do
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        } while (rtc_gpio_get_level(BOTAO) == 1);
    }

    printf("Entrando em modo Light Sleep\n");
    
    uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);

    // Entra em modo Light Sleep
    esp_light_sleep_start();

    esp_sleep_wakeup_cause_t causa = esp_sleep_get_wakeup_cause();
  }

}
void monitoring_task(void *pvParameter)
{
    for(;;){
        // Modificação: Use PRIu32 para imprimir o tamanho da heap corretamente
        ESP_LOGI(TAG, "free heap: %" PRIu32, esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
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
    wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &cb_connection_ok);

    xTaskCreate(leituraSensores, "leitura dos sensores", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
    xTaskCreate(configuraSleep, "configuraSleep", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    
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
    configuraPinos();
    mqtt_app_start();

    filaDeInterrupcao = xQueueCreate(10, sizeof(int));
    xTaskCreate(trataInterrupcaoBotao, "TrataBotao", 2048, NULL, 1, NULL);
    
    gpio_install_isr_service(0);
    gpio_isr_handler_add(botaoFarol, gpio_isr_handler, (void *) botaoFarol);
    gpio_isr_handler_add(botaoVidros, gpio_isr_handler, (void *) botaoVidros);
    gpio_isr_handler_add(botaoTrava, gpio_isr_handler, (void *) botaoTrava);
    //inicializaPwm();    
}