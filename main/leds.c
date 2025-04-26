#include <leds.h>


void tarea_led(void *args)
{
    led_task_t parametros = (led_task_t)args;
    static TickType_t lastEvent;
    gpio_set_direction(parametros->gpio_id_red, GPIO_MODE_OUTPUT);
    gpio_set_direction(parametros->gpio_id_verde, GPIO_MODE_OUTPUT);
    uint8_t bits_mask = parametros->mask_verde | parametros->mask_red;

    lastEvent = xTaskGetTickCount();

    while (1)
    {
        EventBits_t uxBits = (xEventGroupWaitBits(parametros->event_group, bits_mask, pdFALSE, pdFALSE, pdMS_TO_TICKS(50)));
        if ((uxBits & parametros->mask_red) != 0)
        {
            gpio_set_level(parametros->gpio_id_verde, 0);
            gpio_set_level(parametros->gpio_id_red, 1);
        }

        if ((uxBits & parametros->mask_verde) != 0)
        {
            gpio_set_level(parametros->gpio_id_red, 0);
            vTaskDelayUntil(&lastEvent, pdMS_TO_TICKS(parametros->tiempo));
            gpio_set_level(parametros->gpio_id_verde, 1);
            vTaskDelayUntil(&lastEvent, pdMS_TO_TICKS(parametros->tiempo));
            gpio_set_level(parametros->gpio_id_verde, 0);
        }
    }
}


