#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "app_main.c"
void farois(int valor){
    esp_rom_gpio_pad_select_gpio(2);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);
    gpio_set_level(2, valor);
    if(valor){
        printf("Farois ligados\n");
        
    }else{
        printf("Farois desligados\n");
    }
    
}
void travar(){
    // esp_rom_gpio_pad_select_gpio(5);
    // gpio_set_direction(5, GPIO_MODE_OUTPUT);
    // gpio_set_level(5, 1);
    printf("Porta travada\n");
}
void destravar(){
    // esp_rom_gpio_pad_select_gpio(5);
    // gpio_set_direction(5, GPIO_MODE_OUTPUT);
    // gpio_set_level(5, 0);
    printf("Porta destravada\n");
}

void dispararAlarme(){
    while(statusTravado == 1){
        printf("Alarme disparado\n");
    }
}