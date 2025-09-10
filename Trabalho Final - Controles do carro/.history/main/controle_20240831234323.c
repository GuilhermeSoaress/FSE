#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void farois(char valor){
    if(valor == 1){
        printf("Farois ligados\n");
    }else{
        printf("Farois desligados\n");
    }
}