#ifndef _LEDS_H_
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"



typedef struct {
    EventGroupHandle_t event_group;
    uint8_t event_bit;
    gpio_num_t gpio_id_red;
    gpio_num_t gpio_id_verde;
    uint16_t tiempo;
} * led_task_t;


void tarea_led(void * args);

#endif