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
#include "wifi_manager.h"
#include "esp_sleep.h"
#include "esp32/rom/uart.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#define ldr_channel ADC_CHANNEL_6
#define INVALID_ADC_VALUE -1

static int adc_raw[2][10];        
static const char *TAG = "Relampago Marquinhos 2 ";

esp_mqtt_client_handle_t client;
esp_mqtt_client_handle_t client2;
int requestID=0;
int requestID2=0;
int farolStatus = 0;
char attributes[100];
char telemetry[100]; 
char method[50];
char topic[50];
int portaStatus = 0;
int luzInternaStatus = 0;
int botaoTravarStatus = 0;
int travadoStatus = 0;
int luzInternaPwm = 0;
int i =1;

//Inputs
#define botaoIgnicao 12
#define botaoPorta 16
#define botaoBiometria 4 

//Outputs
#define descerVidros 13 
#define buzina 14
#define farol 27 
#define setas 26
#define travaDestrava 25
#define LED_1 19
#define luzInterna 19

char portaStr[10];
char botaoTravarStr[10];
char travadoStatusStr[10];
char farolStatusStr[10];
char luzInternaStr[10];


QueueHandle_t filaDeInterrupcao;

void configuraPinos(){
    esp_rom_gpio_pad_select_gpio(botaoPorta);
    gpio_set_direction(botaoPorta, GPIO_MODE_INPUT);
    gpio_pulldown_en(botaoPorta);
    gpio_pullup_dis(botaoPorta);

    esp_rom_gpio_pad_select_gpio(botaoBiometria);
    gpio_set_direction(botaoBiometria, GPIO_MODE_INPUT);
    gpio_pulldown_en(botaoBiometria);
    gpio_pullup_dis(botaoBiometria);
    
    gpio_set_intr_type(botaoPorta, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(botaoBiometria, GPIO_INTR_HIGH_LEVEL);

    esp_rom_gpio_pad_select_gpio(descerVidros);
    gpio_set_direction(descerVidros, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(buzina);
    gpio_set_direction(buzina, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(farol);
    gpio_set_direction(farol, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(setas);
    gpio_set_direction(setas, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(travaDestrava);
    gpio_set_direction(travaDestrava, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(luzInterna);
    gpio_set_direction(luzInterna, GPIO_MODE_OUTPUT);
     
    
    
}
void inicializaPwm(){
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    esp_err_t timer_status = ledc_timer_config(&timer_config);
    if (timer_status != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao configurar o timer: %s", esp_err_to_name(timer_status));
        return;
    }
    ESP_LOGI(TAG, "Timer configurado com sucesso.");

    ledc_channel_config_t channel_config = {
        .gpio_num = LED_1,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    esp_err_t channel_status = ledc_channel_config(&channel_config);
    if (channel_status != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao configurar o canal: %s", esp_err_to_name(channel_status));
        return;
    }
    ESP_LOGI(TAG, "Canal configurado com sucesso.");
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
        case botaoPorta:
            //printf("Botão Porta pressionado\n");
            if(botaoPorta == 1 && travadoStatus == 1){
                dispararAlarme();
            }
            else if(botaoPorta == 1 && travadoStatus == 0){
                ligarLuzInterna();
                luzInternaStatus = 1;

            }
            else if(botaoPorta == 0 && travadoStatus == 0){
                desligarLuzInterna();
                luzInternaStatus = 0;
            }
            
            portaStatus = estado;
            snprintf(attributes, sizeof(attributes), "{\"porta\": %d, \"luzInterna\": %d, \"travadoStatus\", %d, \"farol\", %d}", portaStatus, luzInternaStatus, travadoStatus, farolStatus);
            mqtt_envia_mensagem("v1/devices/me/attributes", attributes);

            sprintf(portaStr, "%d", portaStatus);
            mqtt_envia_mensagem2("gpios/porta", portaStr);
            
            // sprintf(luzInternaStr, "%d", luzInternaStatus);
            // sprintf(travadoStatusStr, "%d", travadoStatus);
            // sprintf(farolStatusStr, "%d", farolStatus);
            // mqtt_envia_mensagem2("gpios/luzInterna", luzInternaStr);
            // mqtt_envia_mensagem2("gpios/travadoStatus", travadoStatusStr);
            // mqtt_envia_mensagem2("gpios/farol", farolStatusStr);
            break;
        case botaoBiometria:
            printf("Botão Biometria pressionado\n");
            if(estado ==1){
            
                if(travadoStatus == 1){
                    destravar();
                    travadoStatus = 0;
                    snprintf(attributes, sizeof(attributes), "{\"porta\": %d, \"luzInterna\": %d, \"travadoStatus\", %d, \"farol\", %d}", portaStatus, luzInternaStatus, travadoStatus, farolStatus);
                    mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
                    mqtt_envia_mensagem2("gpios/travadoStatus", "0");
                    //vTaskDelay(1000 / portTICK_PERIOD_MS);
                }
                else{
                    travar();
                    travadoStatus = 1;
                    snprintf(attributes, sizeof(attributes), "{\"porta\": %d, \"luzInterna\": %d, \"travadoStatus\", %d, \"farol\", %d}", portaStatus, luzInternaStatus, travadoStatus, farolStatus);
                    mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
                    mqtt_envia_mensagem2("gpios/travadoStatus", "1");
                    //vTaskDelay(1000 / portTICK_PERIOD_MS);
                }
            }

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
        vTaskDelay(1000 / portTICK_PERIOD_MS);
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
    //Get Topic
    // cJSON *topic_item = cJSON_GetObjectItem(json, "topic");
    // if (cJSON_IsString(topic_item) && (topic_item->valuestring != NULL)) {
    //     snprintf(topic, 256, "%s", topic_item->valuestring);
    // } else {
    //     printf("Error getting topic\n");
    //     cJSON_Delete(json);
    //     return 0;
    // }
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
void processaMetodo(const char *data, char *method, char *topic, char *valorTopico) {
    int valor = separaParametros(data, method);
    //printf("Method: %s\n", method);
    printf("Topic:%s\n", topic);
    printf("ValorTopico: %s\n", valorTopico);
    printf("Valor: %d\n", valor);
    //printf("TOPIC=%.*s\r\n", topic_len, topic);
    if(strcmp(topic, "sensores/farol")==0){ 
        if(strcmp(valorTopico, "1") == 0)  valor = 1 
        else valor = 0;
        
        //printf("Entrou no farol\n");
        farolStatus = valor;
        farois(valor);
        sprintf(farolStatusStr, "%d", farolStatus);
        snprintf(attributes, sizeof(attributes), "{\"porta\": %d, \"luzInterna\": %d, \"travadoStatus\", %d, \"farol\", %d}", portaStatus, luzInternaStatus, travadoStatus, farolStatus);
        mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
        mqtt_envia_mensagem2("gpios/farol", farolStatusStr);
    }
    else if(strcmp(topic, "sensores/vidros")==0){
        printf("Entrou no vidros\n");

        if(strcmp(valorTopico,"1")==0){
            fDescerVidros(); 
            sprintf(attributes, "{\"porta\": %d, \"luzInterna\": %d, \"travadoStatus\", %d, \"farol\", %d}", portaStatus, luzInternaStatus, travadoStatus, farolStatus);
            mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
            mqtt_envia_mensagem2("sensore/vidros", "0");
        }
    }
    else if(strcmp(topic,"sensores/trava")==0){
        printf("Entrou no trava\n");

       if(strcmp(valorTopico,"1")==0){
        if(travadoStatus == 0){
            travar();
            travadoStatus = 1;
        }
        else{
            destravar();
            travadoStatus = 0;
        }
        mqtt_envia_mensagem2("sensores/trava", "0");
        // sprintf(travadoStatusStr, "%d", travadoStatus);
        // snprintf(attributes, sizeof(attributes), "{\"porta\": %d, \"luzInterna\": %d, \"travadoStatus\", %d, \"farol\", %d}", portaStatus, luzInternaStatus, travadoStatus, farolStatus);
        // mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
       }
        //mqtt_envia_mensagem2("gpio/travadoStatus", travadoStatusStr);
    }
    else if(strcmp(method, "travadoStatus") == 0){
        (valor == 0) ? destravar() : travar();
        travadoStatus = valor;
        sprintf(travadoStatusStr, "%d", travadoStatus);
        snprintf(attributes, sizeof(attributes), "{\"porta\": %d, \"luzInterna\": %d, \"travadoStatus\", %d, \"farol\", %d}", portaStatus, luzInternaStatus, travadoStatus, farolStatus);
        mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
        mqtt_envia_mensagem2("gpios/travadoStatus", travadoStatusStr);
    }
    else if(strcmp(method, "farol") == 0){
        farois(valor);
        printf("Status do farol %d\n", farolStatus);
        farolStatus = valor;
        sprintf(farolStatusStr, "%d", farolStatus);
        sprintf(attributes, "{\"farol\": %d}", farolStatus);
        mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
        mqtt_envia_mensagem2("gpios/farol", farolStatusStr);
        
    } 
    else if(strcmp(method, "luzInterna")==0){
        printf("Luz Interna %d\n", valor);
        luzInternaPwm = valor;
        sprintf(luzInternaStr, "%d", luzInternaPwm);
        int duty = (int)(valor * 255 / 100);
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        sprintf(attributes, "{\"luzInterna\": %d}", luzInternaPwm);
        mqtt_envia_mensagem("v1/devices/me/attributes", attributes);
        mqtt_envia_mensagem2("gpios/luzInterna", luzInternaStr);
    }
    else
        printf("Método não encontrado\n");
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
            esp_mqtt_client_subscribe(client, "v1/devices/me/rpc/request/+", 0);
            esp_mqtt_client_subscribe(client, "v1/devices/me/attributes/response/+" , 0);
            
            esp_mqtt_client_subscribe(client2, "sensores/farol", 0);
            esp_mqtt_client_subscribe(client2, "sensores/vidros" , 0);
            esp_mqtt_client_subscribe(client2, "sensores/trava" , 0);

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
            //requestID2 = event2->event_id;
             printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
             printf("Event ID: %d\n", requestID);
             printf("DATA=%.*s\r\n", event->data_len, event->data);

            // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            // printf("Event ID: %d\n", requestID2);
            // printf("DATA=%.*s\r\n", event->data_len, event->data);
            sprintf(topic, "%.*s", event->topic_len, event->topic);
            char valorData[5];
            sprintf(valorData, "%.*s", event->data_len, event->data);
            processaMetodo(event->data, method, topic, valorData);
            
                    
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
void cb_connection_ok(void *pvParameter){
    ip_event_got_ip_t* param = (ip_event_got_ip_t*)pvParameter;

    /* transform IP to human readable string */
    char str_ip[16];
    esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);

    ESP_LOGI(TAG, "I have a connection and my IP is %s!", str_ip);
}
void configuraSleep()
{
    esp_rom_gpio_pad_select_gpio(botaoIgnicao);
    gpio_set_direction(botaoIgnicao, GPIO_MODE_INPUT);
    gpio_set_pull_mode(botaoIgnicao, GPIO_PULLUP_ONLY);
    gpio_wakeup_enable(botaoIgnicao, GPIO_INTR_HIGH_LEVEL);
    
    esp_sleep_enable_gpio_wakeup();

  while(true)
  {

    if (rtc_gpio_get_level(botaoIgnicao) == 1)
    {
        printf("Acordando ... \n");
        do
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        } while (rtc_gpio_get_level(botaoIgnicao) == 1);
    }

    printf("Entrando em modo Light Sleep\n");
    
    // Configura o modo sleep somente após completar a escrita na UART para finalizar o printf
    uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);

    //int64_t tempo_antes_de_dormir = esp_timer_get_time();

    // Entra em modo Light Sleep
    esp_light_sleep_start();

    //int64_t tempo_apos_acordar = esp_timer_get_time();

    esp_sleep_wakeup_cause_t causa = esp_sleep_get_wakeup_cause();

    //printf("Dormiu por %lld ms\n", (tempo_apos_acordar - tempo_antes_de_dormir) / 1000);
    //printf("O [%s] me acordou !\n", causa == ESP_SLEEP_WAKEUP_TIMER ? "TIMER" : "BOTÃO");

  }
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
    inicializaPwm();
    //xTaskCreate(enviaEstadoGpio, "enviaEstadoGpio", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);

    filaDeInterrupcao = xQueueCreate(10, sizeof(int));
    xTaskCreate(trataInterrupcaoBotao, "TrataBotao", 4096, NULL, 1 , NULL);
    xTaskCreate(configuraSleep, "configuraSleep", configMINIMAL_STACK_SIZE, NULL, 5, NULL); //thread de light sleep

    gpio_install_isr_service(0);
    gpio_isr_handler_add(botaoPorta, gpio_isr_handler, (void *) botaoPorta);
    gpio_isr_handler_add(botaoBiometria, gpio_isr_handler, (void *) botaoBiometria);
   

}
