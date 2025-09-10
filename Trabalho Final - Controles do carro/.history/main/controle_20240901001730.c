#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void farois(bool valor){
    if(valor){
        printf("Farois ligados\n");
        
    }else{
        printf("Farois desligados\n");
    }
    
}