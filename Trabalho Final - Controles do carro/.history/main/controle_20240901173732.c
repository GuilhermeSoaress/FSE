#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

void farois(int valor){
    esp_rom_gpio_pad_select_gpio(2);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);
    gpio_set_level(2, !valor);
    if(valor){
        printf("Farois ligados\n");
        
    }else{
        printf("Farois desligados\n");
    }
    
}