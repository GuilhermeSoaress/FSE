#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#define luzInterna 19
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
        printf("Alarme disparado\n");
}
void ligarLuzInterna(){
    gpio_set_level(luzInterna, 1);
    printf("Luz Interna ligada\n");
}
void desligarLuzInterna(){
    gpio_set_level(luzInterna, 0);
    printf("Luz Interna desligada\n");
}
void fDescerVidros(){
    printf("Vidros descendo\n");
}