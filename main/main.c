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
#include "ili9341.h"
#include "digitos.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "teclas.h"
#include "leds.h"
#include "time_struct.h"

/* === Macros definitions =========================================================================================== */

#define DIGITO_ANCHO 60
#define DIGITO_ALTO 100
#define DIGITO_ENCENDIDO ILI9341_RED
#define DIGITO_APAGADO 0x3800
#define DIGITO_FONDO ILI9341_BLACK

#define LED_ROJO GPIO_NUM_26
#define LED_VERDE GPIO_NUM_27

#define BOTON1 GPIO_NUM_13
#define BOTON2 GPIO_NUM_32
#define BOTON3 GPIO_NUM_35

#define BOTON_ESTADO 1 << 0
#define BOTON_BORRAR 1 << 1
#define BOTON_PARCIAL 1 << 2
#define BLINK 1 << 3
#define RED 1 << 4
#define CUENTA 1 << 5
//#define REFRESCO_PANTALLA 1 << 6
#define RESET 1 << 6
#define EN_PAUSA 1 << 7
#define PARCIAL 1 << 8


#define QUEUE_LENGTH    10
#define QUEUE_LENGTH_P    3
#define ITEM_SIZE       sizeof(time_struct)

static StaticQueue_t xDisplayQueue;
static StaticQueue_t xDisplayQueue_p;
/* === Private data type declarations =============================================================================== */

typedef struct crono_task
{
    time_struct_t time;
    EventGroupHandle_t event_group;
    QueueHandle_t handler_time;
    QueueHandle_t handler_time_p;
} crono_task;

typedef struct crono_task *crono_task_t;

typedef struct display_task
{
    QueueHandle_t p;  //
    QueueHandle_t t;
    EventGroupHandle_t event_group; 
} display_task;

typedef struct display_task *display_task_t;

/* === Private variable declarations ================================================================================ */

static const char *TAG = "app_main";
// static const char *B2 = "boton_2";

/* === Public function implementation =============================================================================== */

void contar_decima(void *args)
{
    crono_task_t cronometro_p = (crono_task_t)args;
    EventGroupHandle_t _event_group = cronometro_p->event_group;
    QueueHandle_t qHandle = cronometro_p->handler_time;
    QueueHandle_t qHandle_p = cronometro_p->handler_time;
    TickType_t lastEvent;
    time_struct_t cronometro = cronometro_p->time;
    time_struct buffer;
    time_cero(cronometro);
    lastEvent = xTaskGetTickCount();

    while (1)
    {
        EventBits_t wBits = xEventGroupWaitBits(_event_group, CUENTA | RESET | EN_PAUSA, pdFALSE, pdFALSE, portMAX_DELAY);
      //  ESP_LOGI(TAG, "Estado de los bits en contar_decima: %lu", wBits);
        if ((wBits & CUENTA) != 0)
        {
            // al volver de la pausa, reinicio el lastEvent
            if ((wBits & EN_PAUSA) != 0)
            {
                xEventGroupClearBits(_event_group, EN_PAUSA);
                lastEvent = xTaskGetTickCount();
            }
            // ESP_LOGI(TAG, "CUENTA activado - Tick actual: %lu, Last Event: %lu", xTaskGetTickCount(), lastEvent);
            ESP_LOGI(TAG, "time: %d %d %d.%d", cronometro->centena,
                                        cronometro->decena,cronometro->unidad,
                                        cronometro->decima
                                );
            time_tick(cronometro);
            xQueueSend(qHandle,cronometro,portMAX_DELAY);
        }
        if ((wBits & RESET) != 0)
        {
            time_cero(cronometro);
            xEventGroupClearBits(_event_group, RESET);
            xQueueSend(qHandle,cronometro,portMAX_DELAY);
        }
        if ((wBits & PARCIAL) != 0)
        {
            xEventGroupClearBits(_event_group, PARCIAL);
            xQueueSend(qHandle_p,cronometro,portMAX_DELAY);
        }
        vTaskDelayUntil(&lastEvent, pdMS_TO_TICKS(100));
    }
}

void dibujar_pantalla(void *args)
{
    display_task_t display_arg = (display_task_t)args;
   // EventGroupHandle_t _event_group = display_arg->event_group;
    QueueHandle_t queue_t = display_arg->t;
    QueueHandle_t queue_parcial = display_arg->p;

    ILI9341Init();
    ILI9341Rotate(ILI9341_Landscape_1);

    panel_t segundos = CrearPanel(30, 60, 3, DIGITO_ALTO, DIGITO_ANCHO, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t decimas = CrearPanel(230, 60, 1, DIGITO_ALTO, DIGITO_ANCHO, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    time_struct parcial[3];
    time_struct tiempo;
    int unidad_ant = 0;
    int decena_ant = 0;
    int centena_ant = 0;
    int unidad_act = 0;
    int decena_act = 0;
    int centena_act = 0;
    int decima_ant = 0;
    int decima_act = 0;
    DibujarDigito(segundos, 2, 0);
    DibujarDigito(segundos, 1, 0);
    DibujarDigito(segundos, 0, 0);
    DibujarDigito(decimas, 0, 0);
    ILI9341DrawFilledCircle(220, 150, 5, DIGITO_ENCENDIDO);
    while (1)
    {
        //if (xEventGroupWaitBits(_event_group, REFRESCO_PANTALLA, pdTRUE, pdFALSE, portMAX_DELAY))
      //  xQueueReceive(queue_t,&tiempo,portMAX_DELAY);
        if( xQueueReceive(queue_t,
                           &( tiempo ),
                           ( TickType_t ) 50 ) == pdPASS )
        {
                        ESP_LOGI(TAG, "time: %d %d %d.%d", tiempo.centena,
                                        tiempo.decena,tiempo.unidad,
                                        tiempo.decima
                                );
            unidad_act = tiempo.unidad;
            decena_act = tiempo.decena;
            centena_act = tiempo.centena;
            decima_act = tiempo.decima;
            //   ESP_LOGI(TAG, "Valores en dibujar_pantalla: %d, %d, %d, %d", unidad_act, decena_act, centena_act, decima);
        }
        if (unidad_act != unidad_ant)
            DibujarDigito(segundos, 2, unidad_act);
        if (decena_act != decena_ant)
            DibujarDigito(segundos, 1, decena_act);
        if (centena_act != centena_ant)
            DibujarDigito(segundos, 0, centena_act);
        if (decima_act != decima_ant)
            DibujarDigito(decimas, 0, decima_act);
        decima_ant = decima_act;
        unidad_ant = unidad_act;
        decena_ant = decena_act;
        centena_ant = centena_act;

    }
}

void cambia_estado(void *args)
{
    EventGroupHandle_t _event_group = (EventGroupHandle_t)args;
    while (1)
    {
        EventBits_t wBits = (xEventGroupWaitBits(_event_group, BOTON_ESTADO | CUENTA, pdFALSE, pdFALSE, portMAX_DELAY));
        {
            // ESP_LOGI(TAG, "Estado de los bits en cambia_estado: %lu", wBits);
            if (((wBits & CUENTA) != 0) && ((wBits & BOTON_ESTADO) != 0))
            {
                xEventGroupClearBits(_event_group, CUENTA);
                xEventGroupClearBits(_event_group, BOTON_ESTADO);
                xEventGroupClearBits(_event_group, BOTON_BORRAR);
                xEventGroupClearBits(_event_group, BLINK);
                xEventGroupSetBits(_event_group, EN_PAUSA);
                xEventGroupSetBits(_event_group, RED);
            }
            else if ((wBits & BOTON_ESTADO) != 0)
            {
                xEventGroupSetBits(_event_group, BLINK);
                xEventGroupSetBits(_event_group, CUENTA);

                xEventGroupClearBits(_event_group, BOTON_ESTADO);
                xEventGroupClearBits(_event_group, RED);
                xEventGroupClearBits(_event_group, BOTON_BORRAR);

                vTaskDelay(pdMS_TO_TICKS(20));
            }
        }
    }
}

void borrar(void *args)
{
    EventGroupHandle_t _event_group = (EventGroupHandle_t)args;
    while (1)
    {
        EventBits_t wBits = (xEventGroupWaitBits(_event_group, BOTON_BORRAR | CUENTA, pdFALSE, pdFALSE, portMAX_DELAY));
        {
            if ((wBits & CUENTA) != 0)
            {
                continue;
            }
            else if ((wBits & BOTON_BORRAR) != 0) // solo borrar si la cuenta está detenida
            {
                xEventGroupClearBits(_event_group, BOTON_BORRAR);
                xEventGroupSetBits(_event_group, RESET);
            }
        }
    }
}

void tomar_parcial(void *args)
{
    EventGroupHandle_t _event_group = (EventGroupHandle_t)args;
    while (1)
    {
        EventBits_t wBits = (xEventGroupWaitBits(_event_group, BOTON_PARCIAL | CUENTA, pdFALSE, pdTRUE, portMAX_DELAY));
        {
            ESP_LOGI(TAG, "Estado de los bits en tomar_parcial: %lu", wBits);
        }
    }
}

void app_main(void)
{
    key_task_t key_args;
    led_task_t leds_args;
    crono_task_t crono_args; 
    display_task_t display_args;
    EventGroupHandle_t event_group = xEventGroupCreate();
    uint8_t buffer_display[ QUEUE_LENGTH * ITEM_SIZE ];
    uint8_t buffer_parciales[ QUEUE_LENGTH_P * ITEM_SIZE ];
    QueueHandle_t q_pantalla = xQueueCreateStatic(QUEUE_LENGTH,
                                                 ITEM_SIZE,
                                                 buffer_display,
                                                 &xDisplayQueue);
    QueueHandle_t q_parciales = xQueueCreateStatic(QUEUE_LENGTH_P,
                                                ITEM_SIZE, 
                                                buffer_parciales,
                                                &xDisplayQueue_p);
    //eventos del estado inicial
    xEventGroupSetBits(event_group, RED);
    xEventGroupSetBits(event_group, EN_PAUSA);


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

        leds_args = malloc(sizeof(led_task));
        leds_args->event_group = event_group;
        leds_args->mask_red = RED;
        leds_args->mask_verde = BLINK;
        leds_args->tiempo = 200;
        leds_args->gpio_id_red = LED_ROJO;
        leds_args->gpio_id_verde = LED_VERDE;

        if (xTaskCreate(tarea_led, "led", 1024, leds_args, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear staus ");

        if (xTaskCreate(cambia_estado, "status", 2 * 1024, event_group, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear staus ");

        if (xTaskCreate(borrar, "borrar", 1024, event_group, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear borrado ");

        if (xTaskCreate(tomar_parcial, "parcial", 2 * 1024, event_group, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear parcial ");

        crono_args = malloc(sizeof(crono_task));
        crono_args->time = malloc(sizeof(time_struct));
        crono_args->event_group = event_group;
        crono_args->handler_time = q_pantalla;
        crono_args->handler_time_p = q_parciales;
        if (xTaskCreate(contar_decima, "contar", 3 * 1024, crono_args, tskIDLE_PRIORITY + 3, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear contar");

        display_args = malloc(sizeof(display_task));
        display_args->t = q_pantalla;
        display_args->p = q_parciales;
        display_args->event_group = event_group; 
        if (xTaskCreate(dibujar_pantalla, "pantalla", 5*1024, display_args, tskIDLE_PRIORITY + 2, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear pantalla");
    }
    else
        ESP_LOGE(TAG, "Fallo al crear Eventos ");
}

/* === End of documentation ========================================================================================= */

/** @} End of module definition for doxygen */
