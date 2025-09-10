#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#import "app_main.c"

void farois(char valor){
    if(valor == 1){
        printf("Farois ligados\n");
        
    }else{
        printf("Farois desligados\n");
    }
    farol_status = valor;
}