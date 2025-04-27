#ifndef _LEDS_H_
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"



typedef struct led_task{
    EventGroupHandle_t event_group;
    uint8_t mask_verde;
    uint8_t mask_red;
    gpio_num_t gpio_id_red;
    gpio_num_t gpio_id_verde;
    uint16_t tiempo;
} led_task;
typedef struct led_task *led_task_t;


void tarea_led(void * args);

#endif