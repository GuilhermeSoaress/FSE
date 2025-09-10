#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void farois(char *valor){
    if(valor){
        printf("Farois ligados\n");
        
    }else{
        printf("Farois desligados\n");
    }
    
}