#ifndef _TECLAS_H_
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"



typedef struct {
    EventGroupHandle_t event_group;
    uint8_t event_bit;
    gpio_num_t gpio_id;
} * key_task_t;


void tarea_tecla(void * args);


#endif