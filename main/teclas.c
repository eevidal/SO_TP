#include "teclas.h"

void tarea_tecla(void * args){  
    key_task_t parametros = (key_task_t)args;
    gpio_set_direction(parametros->gpio_id, GPIO_MODE_INPUT);
    gpio_pullup_en(parametros->gpio_id);
    int level;
    level = gpio_get_level(parametros->gpio_id);

    while(1){
     

     while (level == 1)
        {
            level = gpio_get_level(parametros->gpio_id);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

     xEventGroupSetBits(parametros->event_group, parametros->event_bit);    //el evento es que se presionó el botón    

     while (level == 0)
        {
            level = gpio_get_level(parametros->gpio_id);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

    }

}

