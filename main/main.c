/************************************************************************************************
Copyright (c) 2025, Erica Vidal <ericavidal@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

SPDX-License-Identifier: MIT
*************************************************************************************************/

/* === Headers files inclusions ==================================================================================== */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "display.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "teclas.h"
#include "leds.h"
#include "time_struct.h"
#include "string.h"
#include "mode_op.h"
/* === Macros definitions =========================================================================================== */

#define LED_ROJO GPIO_NUM_26
#define LED_VERDE GPIO_NUM_27

#define BOTON1 GPIO_NUM_13
#define BOTON2 GPIO_NUM_32
#define BOTON3 GPIO_NUM_35
#define BOTON4 GPIO_NUM_34

#define BOTON_ESTADO 1 << 0
#define BOTON_BORRAR 1 << 1
#define BOTON_PARCIAL 1 << 2

#define BOTON_1 1 << 0
#define BOTON_2 1 << 1
#define BOTON_3 1 << 2

#define BLINK 1 << 3
#define RED 1 << 4
#define CUENTA 1 << 5
#define RESET 1 << 6
#define TOMAR_PARCIAL 1 << 7
#define EN_PAUSA 1 << 8

#define BOTON_MODO 1 << 9

#define QUEUE_LENGTH 10

#define QUEUE_LENGTH_C 10
#define ITEM_SIZE sizeof(time_struct)

/* === Private data type declarations =============================================================================== */

typedef struct crono_task
{
    time_struct_t time;

    EventGroupHandle_t event_group;
    QueueHandle_t handler_time;
    int estado; 
} crono_task;

typedef struct crono_task *crono_task_t;

typedef struct clock_task
{
    time_clock_t clock;
    clock_settings_t alarm;
    time_struct_t time; // crono
    bool alarm_seted;
    EventGroupHandle_t event_group;
    int selected; // 0 -> hr, 1-> min, 2-> seg , 3 -> dia, 4->mes, 5 -> año
    QueueHandle_t handler_clock, handler_alarm, handler_crono, handler_conf;
    modos_t modo;
} clock_task;

typedef struct clock_task *clock_task_t;

/* === Private variable declarations ================================================================================ */

static const char *TAG = "app_main";
static const char *TAG_ALARM = "dispara_alarma";

int is_one(long n, int b)
{
    return (n >> b) & 1;
}

void printbin(unsigned long n)
{
    for (int b = 31; b >= 0; b--)
        printf("%d", is_one(n, b));
    printf("\n");
}

/* === Public function implementation =============================================================================== */

void contar_decima(void *args)
{
    crono_task_t cronometro_p = (crono_task_t)args;
    EventGroupHandle_t _event_group = cronometro_p->event_group;
    QueueHandle_t qHandle = cronometro_p->handler_time;
    TickType_t lastEvent;
    time_struct_t cronometro = cronometro_p->time;
    time_cero(cronometro);
    lastEvent = xTaskGetTickCount();

    while (1)
    {
        // desición de diseño: esto es independiente del modo
        EventBits_t wBits = xEventGroupWaitBits(_event_group, CUENTA | RESET | EN_PAUSA, pdFALSE, pdFALSE, portMAX_DELAY);
        //     ESP_LOGI(TAG, "Estado de los bits en contar_decima: %lu", wBits);

        if ((wBits & CUENTA) != 0)
        {
            // al volver de la pausa, reinicio el lastEvent
            if ((wBits & EN_PAUSA) != 0)
            {
                xEventGroupClearBits(_event_group, EN_PAUSA);
                lastEvent = xTaskGetTickCount();
            }
            // ESP_LOGI(TAG, "CUENTA activado - Tick actual: %lu, Last Event: %lu", xTaskGetTickCount(), lastEvent);
            /*  ESP_LOGI(TAG, "time: %d %d %d.%d", cronometro->centena,
                                          cronometro->decena,cronometro->unidad,
                                          cronometro->decima
                                  );*/
            time_tick(cronometro);
            switch (wBits & (MODOS))
            {
            case MODO_CLOCK_CONF:
                break;
            case MODO_CLOCK:
                break;
            case MODO_ALARM:
                break;
            case MODO_ALARM_CONF:
                break;
            case MODO_CRONO:
                xQueueSend(qHandle, cronometro, portMAX_DELAY);
                break;
            default:
                break;
            }
        }
        if ((wBits & RESET) != 0)
        {
            time_cero(cronometro);
            switch (wBits & (MODOS))
            {
            case MODO_CLOCK_CONF:
                break;
            case MODO_CLOCK:
                break;
            case MODO_ALARM:
                break;
            case MODO_ALARM_CONF:
                break;
            case MODO_CRONO:
                xQueueSend(qHandle, cronometro, portMAX_DELAY);
                break;
            default:
                break;
            }
            xQueueSend(qHandle, cronometro, portMAX_DELAY);
        }

        vTaskDelayUntil(&lastEvent, pdMS_TO_TICKS(100));
    }
}
void contar_segundos(void *args)
{
    clock_task_t clock_p = (clock_task_t)args;
    TickType_t lastEvent;
    time_clock_t clock = clock_p->clock;
    EventGroupHandle_t _event_group = clock_p->event_group;
    QueueHandle_t qHandle = clock_p->handler_clock;
    QueueHandle_t qHandle_alarm = clock_p->handler_alarm;

    lastEvent = xTaskGetTickCount();
    while (1)
    {
        // el tiempo pasa siempre excepto cuando estoy configurando el reloj o la alarma
        EventBits_t wBits = xEventGroupWaitBits(_event_group, MODOS, pdFALSE, pdFALSE, portMAX_DELAY);

        if (!((wBits & MODO_CLOCK_CONF) || (wBits & MODO_ALARM_CONF)))
        {
            clock_tick(clock);
        }
        switch (wBits & (MODOS))
        {
        case MODO_CLOCK_CONF:
        case MODO_CLOCK:
            xQueueSend(qHandle, clock, pdMS_TO_TICKS(10)); // MODO CLOCK, envío el tiempo a la pantalla
            break;
        case MODO_CRONO:
            break;
        case MODO_ALARM_CONF:
            xQueueSend(qHandle_alarm, clock_p->alarm, pdMS_TO_TICKS(10));    
            break;
        case MODO_ALARM:
            xQueueSend(qHandle, clock, pdMS_TO_TICKS(10));
            break;
        default:
            break;
        }
        //  ESP_LOGI(TAG, "¡TICK!");

        vTaskDelayUntil(&lastEvent, pdMS_TO_TICKS(1000)); // cuenta segundos
    }
}
// void cambia_estado(void *args)

void cambiar_campo(int selected)
{
    selected++;
    if (selected == 6)
        selected = 0;
    ESP_LOGI(TAG, "Selected: %d", selected);
}
void tarea_b1(void *args)
{
    clock_task_t clock_p = (clock_task_t)args;
    EventGroupHandle_t _event_group = clock_p->event_group;
    QueueHandle_t qHandle_campo = clock_p->handler_conf;
    int selected;

    while (1)
    {
        EventBits_t wBits = (xEventGroupWaitBits(_event_group, BOTON_1 | CUENTA, pdFALSE, pdFALSE, portMAX_DELAY));
        {

            switch (wBits & (MODOS))
            {
            case MODO_CLOCK_CONF:

            if ((wBits & BOTON_1) != 0)
            {
                ESP_LOGI(TAG, "Estado de los bits en cambia_campo: %lu", wBits);
                printbin(wBits);
                clock_p->selected = (clock_p->selected + 1) % 6;
                xEventGroupClearBits(_event_group, BOTON_1);
                selected = clock_p->selected;
                xQueueSend(qHandle_campo, &selected, pdMS_TO_TICKS(10));
            }
            break;
            case MODO_ALARM_CONF:

                if ((wBits & BOTON_1) != 0)
                {
                    ESP_LOGI(TAG, "Estado de los bits en cambia_campo: %lu", wBits);
                    printbin(wBits);
                    clock_p->alarm->select = (clock_p->alarm->select + 1) % 6;
                    xEventGroupClearBits(_event_group, BOTON_1);
                    selected = clock_p->alarm->select;
                    xQueueSend(qHandle_campo, &selected, pdMS_TO_TICKS(10));
                }
                break;

            case MODO_CRONO:
                // ESP_LOGI(TAG, "Estado de los bits en cambia_estado: %lu", wBits);
                if (((wBits & CUENTA) != 0) && ((wBits & BOTON_1) != 0))
                {
                    xEventGroupClearBits(_event_group, CUENTA);
                    xEventGroupClearBits(_event_group, BOTON_ESTADO); // BOTON_1
                    xEventGroupClearBits(_event_group, BOTON_BORRAR); //BOTON_2
                    xEventGroupClearBits(_event_group, BLINK);
                    xEventGroupSetBits(_event_group, EN_PAUSA);
                    xEventGroupSetBits(_event_group, RED);
                }
                else if ((wBits & BOTON_ESTADO) != 0)
                {
                    xEventGroupSetBits(_event_group, BLINK);
                    xEventGroupSetBits(_event_group, CUENTA);
                    xEventGroupClearBits(_event_group, BOTON_ESTADO); // BOTON_1
                    xEventGroupClearBits(_event_group, RED);
                    xEventGroupClearBits(_event_group, BOTON_BORRAR);

                    vTaskDelay(pdMS_TO_TICKS(20));
                }
                break;
            default:
                xEventGroupClearBits(_event_group, BOTON_1);
                break;
            }
        }
    }
}

// void borrar(void *args)
void tarea_b2(void *args)
{
    clock_task_t clock_p = (clock_task_t)args;
    EventGroupHandle_t _event_group = clock_p->event_group;
    QueueHandle_t qHandle_clock = clock_p->handler_clock;
    QueueHandle_t qHandle_alarm = clock_p->handler_alarm;
    while (1)
    {

        // borrar en modo cronometro
        EventBits_t wBits = (xEventGroupWaitBits(_event_group, BOTON_2 | CUENTA, pdFALSE, pdFALSE, portMAX_DELAY)); // 5 min
        {

            switch (wBits & (MODOS))
            {
            case MODO_CLOCK_CONF:

                if ((wBits & BOTON_2) != 0)
                {
                    ESP_LOGI(TAG, "¡B2!");
                    printbin(wBits);
                    clock_decrementar_campo(clock_p->clock, clock_p->selected);
                    ESP_LOGI(TAG, "decrementa : %d", clock_p->clock->hr);
                    xEventGroupClearBits(_event_group, BOTON_2);
                    xQueueSend(qHandle_clock, clock_p->clock, pdMS_TO_TICKS(10));
                }
                break;

            case MODO_ALARM_CONF:
                if ((wBits & BOTON_2) != 0)
                {
                    ESP_LOGI(TAG, "¡B2! CONF ALARM");
                    printbin(wBits);
                    clock_decrementar_campo(clock_p->alarm->t, clock_p->alarm->select);
                    xEventGroupClearBits(_event_group, BOTON_2);
                    xQueueSend(qHandle_alarm, clock_p->alarm, pdMS_TO_TICKS(10));
                }
                break;
            case MODO_ALARM: // alarma sonando
                if ((wBits & BOTON_2) != 0)
                {
                    clock_p->alarm_seted = !clock_p->alarm_seted;
                    xEventGroupClearBits(_event_group, MODO_ALARM);
                    xEventGroupClearBits(_event_group, BOTON_2);
                    xEventGroupClearBits(_event_group, BLINK);
                    switch (clock_p->modo)
                    {
                    case CLOCK:
                        xEventGroupSetBits(_event_group, MODO_CLOCK);
                        break;
                    case CLOCK_CONF:
                        xEventGroupSetBits(_event_group, MODO_CLOCK_CONF);
                        break;
                    case ALARM_CONF:
                        xEventGroupSetBits(_event_group, MODO_ALARM_CONF);
                        break;
                    case CRONO:
                        xEventGroupSetBits(_event_group, MODO_CRONO);
                        break;

                    default:
                        break;
                    }
                }
                break;
            case MODO_CRONO:
                if ((wBits & CUENTA) != 0)
                {
                    continue;
                }
                else if ((wBits & BOTON_2) != 0) // solo borrar si la cuenta está detenida
                {
                    xEventGroupClearBits(_event_group, BOTON_2);
                    xEventGroupSetBits(_event_group, RESET);
                }
                break;
            default:
                xEventGroupClearBits(_event_group, BOTON_2);
                break;
            }
        }
    }
}

// void tomar_parcial(void *args)
void tarea_b3(void *args)
{
    clock_task_t clock_p = (clock_task_t)args;
    EventGroupHandle_t _event_group = clock_p->event_group;
    QueueHandle_t qHandle = clock_p->handler_clock;
    QueueHandle_t qHandle_alarm = clock_p->handler_alarm;
    
    while (1)
    {
        EventBits_t wBits = (xEventGroupWaitBits(_event_group, BOTON_3 | CUENTA, pdFALSE, pdFALSE, portMAX_DELAY));

        switch (wBits & (MODOS))
        {
        case MODO_CLOCK_CONF:
            if ((wBits & BOTON_3) != 0)
            {
                clock_incrementar_campo(clock_p->clock, clock_p->selected);
                xEventGroupClearBits(_event_group, BOTON_3);
                xQueueSend(qHandle, clock_p->clock, pdMS_TO_TICKS(10));
            }

            break;
        case MODO_ALARM:
            clock_incrementar_min(clock_p->alarm->t, 5);
            xEventGroupClearBits(_event_group, MODO_ALARM);
            xEventGroupClearBits(_event_group, BOTON_3);
            xEventGroupSetBits(_event_group, MODO_CLOCK);
            break;
        case MODO_ALARM_CONF:
            if ((wBits & BOTON_3) != 0)
            {
                ESP_LOGI(TAG, "¡B3! ALARM");
                printbin(wBits);
                clock_incrementar_campo(clock_p->alarm->t, clock_p->alarm->select);
                xEventGroupClearBits(_event_group, BOTON_3);
                xQueueSend(qHandle_alarm, clock_p->alarm, pdMS_TO_TICKS(10));
            }
            break;
        case MODO_CRONO:
            if (((wBits & CUENTA) & (wBits & BOTON_PARCIAL)) != 0)
            {
                printbin(wBits);
                xEventGroupSetBits(_event_group, TOMAR_PARCIAL);
                xEventGroupClearBits(_event_group, BOTON_PARCIAL); // BOTON_3
            }
            break;
        default:
            xEventGroupClearBits(_event_group, BOTON_3);
            break;
        }
    }
}

/* */
void cambia_modo(void *args)
{
    EventGroupHandle_t _event_group = (EventGroupHandle_t)args;
    while (1)
    {

        EventBits_t wBits = (xEventGroupWaitBits(_event_group, BOTON_MODO, pdFALSE, pdFALSE, portMAX_DELAY));
        ESP_LOGI(TAG, "Estado de los bits en cambia_modo: %lu", (wBits & (MODOS)));
        switch (wBits & (MODOS))
        {
        // Reloj ->Reloj Config ->Alarma Config-> Crónometro
        case MODO_CLOCK:
            if ((wBits & BOTON_MODO) != 0)
            {
                xEventGroupClearBits(_event_group, MODO_CLOCK);
                xEventGroupSetBits(_event_group, MODO_CLOCK_CONF);
                xEventGroupClearBits(_event_group, BOTON_MODO);
                xEventGroupSetBits(_event_group, CAMBIO_MODO);
                ESP_LOGI(TAG, "¡CAMBIA A MODO CLOCK_CONF!");
            }
            break;

        case MODO_CLOCK_CONF:
            if ((wBits & BOTON_MODO) != 0)
            {
                xEventGroupClearBits(_event_group, MODO_CLOCK_CONF);
                xEventGroupSetBits(_event_group, MODO_ALARM_CONF);
                xEventGroupClearBits(_event_group, BOTON_MODO);
                xEventGroupSetBits(_event_group, CAMBIO_MODO);
                ESP_LOGI(TAG, "¡CAMBIA A MODO ALARM CONF!");
            }
            break;

        case MODO_ALARM: // alarma sonando
            xEventGroupClearBits(_event_group, BOTON_MODO);
            break;
        case MODO_ALARM_CONF:
            if ((wBits & BOTON_MODO) != 0)
            {
                xEventGroupClearBits(_event_group, MODO_ALARM_CONF);
                xEventGroupSetBits(_event_group, MODO_CRONO);
                xEventGroupClearBits(_event_group, BOTON_MODO);
                xEventGroupSetBits(_event_group, CAMBIO_MODO);
                ESP_LOGI(TAG, "¡CAMBIA A MODO CRONO!");
            }
            break;
        case MODO_CRONO: // CRONO
            if ((wBits & BOTON_MODO) != 0)
            {
                xEventGroupClearBits(_event_group, MODO_CRONO);
                xEventGroupSetBits(_event_group, MODO_CLOCK);
                xEventGroupClearBits(_event_group, BOTON_MODO);
                xEventGroupSetBits(_event_group, CAMBIO_MODO);
                xEventGroupClearBits(_event_group, BLINK);
                xEventGroupClearBits(_event_group, RED);

                ESP_LOGI(TAG, "¡CAMBIA A MODO CLOCK!");
            }
            break;
        default:
            break;
        }
    }
}

void dispara_alarma(void *args)
{
    clock_task_t clock_p = (clock_task_t)args;
    time_clock_t clock = clock_p->clock;
    time_clock_t alarm = clock_p->alarm->t;
    EventGroupHandle_t _event_group = clock_p->event_group;
    TickType_t last_check = xTaskGetTickCount();
    const TickType_t check_interval = pdMS_TO_TICKS(500); // chequear cada 1/2 seg.

    while (1)
    {
        vTaskDelayUntil(&last_check, check_interval);
        EventBits_t wBits = (xEventGroupWaitBits(_event_group, MODOS, pdFALSE, pdFALSE, 0));
        if (clock_p->alarm_seted)
        {
            if (clock->hr == alarm->hr &&
                clock->min == alarm->min &&
                clock->sec == alarm->sec &&
                clock->day == alarm->day &&
                clock->month == alarm->month &&
                clock->year == alarm->year)
            {
                if ((wBits & (MODO_ALARM)) == 0)
                {
                    xEventGroupSetBits(_event_group, BLINK);
                    xEventGroupSetBits(_event_group, MODO_ALARM);

                    ESP_LOGI(TAG_ALARM, "¡ALARMA SONANDO!");
                    switch (wBits & (MODOS))
                    {
                    case MODO_CLOCK:
                        clock_p->modo = CLOCK;
                        break;
                    case MODO_CLOCK_CONF:
                        clock_p->modo = CLOCK_CONF;
                        break;
                    case MODO_CRONO:
                        clock_p->modo = CRONO;
                        break;
                    case MODO_ALARM_CONF:
                        clock_p->modo = ALARM_CONF;
                        break;
                    default:
                        break;
                    }
                    xEventGroupClearBits(_event_group, MODOS);
                }
            }
        }
    }
}

void app_main(void)
{
    key_task_t key_args;
    led_task_t leds_args;
    crono_task_t crono_args;
    clock_task_t clock_args;
    display_task_t display_args;
    EventGroupHandle_t event_group = xEventGroupCreate();

    static StaticQueue_t xDisplayQueue_crono;
    static StaticQueue_t xDisplayQueue_alarm;        // clock + campo
    static StaticQueue_t xDisplayQueue_clock;        // clock
    static StaticQueue_t xDisplayQueue_clock_config; // campo

    uint8_t buffer_display[QUEUE_LENGTH * ITEM_SIZE];

    uint8_t buffer_display_c[QUEUE_LENGTH_C * sizeof(time_clock)];
    uint8_t buffer_display_a[QUEUE_LENGTH_C * sizeof(clock_settings)];
    uint8_t buffer_display_conf[QUEUE_LENGTH_C * sizeof(int)];

    QueueHandle_t q_crono = xQueueCreateStatic(QUEUE_LENGTH,
                                               ITEM_SIZE,
                                               buffer_display,
                                               &xDisplayQueue_crono);
    QueueHandle_t q_clock = xQueueCreateStatic(QUEUE_LENGTH,
                                               sizeof(time_clock),
                                               buffer_display_c,
                                               &xDisplayQueue_clock);

    QueueHandle_t q_clock_conf = xQueueCreateStatic(QUEUE_LENGTH_C,
                                                    sizeof(int),
                                                    buffer_display_conf,
                                                    &xDisplayQueue_clock_config);
    QueueHandle_t q_alarm = xQueueCreateStatic(QUEUE_LENGTH_C,
                                               sizeof(clock_settings),
                                               buffer_display_a,
                                               &xDisplayQueue_alarm);
    // eventos del estado inicial
    //  xEventGroupSetBits(event_group, RED);
    xEventGroupSetBits(event_group, MODO_CLOCK);

    if (event_group)
    {
        key_args = malloc(sizeof(key_task));
        key_args->event_group = event_group;
        key_args->gpio_id = BOTON1;
        key_args->event_bit = BOTON_ESTADO;
        if (xTaskCreate(tarea_tecla, "boton1", 1024, key_args, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear boton1 ");

        key_args = malloc(sizeof(key_task));
        key_args->event_group = event_group;
        key_args->gpio_id = BOTON2;
        key_args->event_bit = BOTON_BORRAR;
        if (xTaskCreate(tarea_tecla, "boton2", 1024, key_args, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear boton2");

        key_args = malloc(sizeof(key_task));
        key_args->event_group = event_group;
        key_args->gpio_id = BOTON3;
        key_args->event_bit = BOTON_PARCIAL;
        if (xTaskCreate(tarea_tecla, "boton3", 1024, key_args, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear boton3 ");

        key_args = malloc(sizeof(key_task));
        key_args->event_group = event_group;
        key_args->gpio_id = BOTON4;
        key_args->event_bit = BOTON_MODO;
        if (xTaskCreate(tarea_tecla, "boton4", 1024, key_args, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear boton4 ");

        leds_args = malloc(sizeof(led_task));
        leds_args->event_group = event_group;
        leds_args->mask_red = RED;
        leds_args->mask_verde = BLINK;
        leds_args->tiempo = 200;
        leds_args->gpio_id_red = LED_ROJO;
        leds_args->gpio_id_verde = LED_VERDE;

        if (xTaskCreate(tarea_led, "led", 1024, leds_args, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear status ");

        clock_args = malloc(sizeof(clock_task));
        clock_args->alarm_seted = false;
        clock_args->selected = 0;
        clock_args->clock = malloc(sizeof(time_clock));
        clock_args->alarm = malloc(sizeof(clock_settings));
        clock_args->alarm->select = 0;
        clock_init(clock_args->clock); /* reference time*/
        clock_alarm_init(clock_args->alarm->t);
        clock_args->event_group = event_group;

        clock_args->modo = CLOCK;
        // handler_clock, handler_alarm, handler_crono, handler_conf
        clock_args->handler_clock = q_clock;
        clock_args->handler_alarm = q_alarm;
        clock_args->handler_crono = q_crono;
        clock_args->handler_conf = q_clock_conf;
        if (xTaskCreate(contar_segundos, "clock", 10 * 1024, clock_args, tskIDLE_PRIORITY + 4, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear contar");

        if (xTaskCreate(tarea_b1, "status", 10 * 1024, clock_args, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear status ");

        if (xTaskCreate(tarea_b2, "borrar", 10 * 1024, clock_args, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear borrado ");

        if (xTaskCreate(tarea_b3, "parcial", 20 * 1024, clock_args, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear parcial ");

        if (xTaskCreate(cambia_modo, "modo", 2 * 1024, event_group, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear parcial ");
        crono_args = malloc(sizeof(crono_task));
        crono_args->time = malloc(sizeof(time_struct));
        crono_args->event_group = event_group;
        crono_args->handler_time = q_crono;
        crono_args->estado = 0;
        if (xTaskCreate(contar_decima, "contar", 3 * 1024, crono_args, tskIDLE_PRIORITY + 3, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear contar");

        display_args = malloc(sizeof(display_task));
        display_args->qcrono = q_crono;
        display_args->qclock = q_clock;
        display_args->qalarm = q_alarm;
        display_args->qconf = q_clock_conf;
        display_args->selected = 0;
        display_args->alarm_set = false;
        display_args->event_group = event_group;
        display_args->parcial_bits = TOMAR_PARCIAL;
        display_args->reset_bits = RESET;
        if (xTaskCreate(dibujar_pantalla, "pantalla", 50 * 1024, display_args, tskIDLE_PRIORITY + 2, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear pantalla");
    }
    else
        ESP_LOGE(TAG, "Fallo al crear Eventos ");
}

/* === End of documentation ========================================================================================= */

/** @} End of module definition for doxygen */
