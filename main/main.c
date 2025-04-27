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
#define REFRESCO_PANTALLA 1 << 6
#define RESET 1 << 7
#define EN_PAUSA 1<<8
/* === Private data type declarations =============================================================================== */

typedef struct blink
{
    gpio_num_t led;
    uint16_t tiempo;
} *blink_t;

typedef enum
{
    UNIDAD = 0,
    DECENA = 1,
    CENTENA = 2
} digito_t;

typedef enum
{
    OFF = 0,
    ON = 1,
} state_t;

/* === Private variable declarations ================================================================================ */

static const char *TAG = "app_main";
// static const char *B2 = "boton_2";

/* === Public variable definitions ================================================================================== */

volatile int decima = 0;
int unidad = 0;
int decena = 0;
int centena = 0;

/* === Private function implementation ============================================================================== */

void incrementar_segundo(digito_t digito_inicial)
{
    digito_t digito_actual = digito_inicial;
    bool carry = true;
    while (carry)
    {
        carry = false;
        switch (digito_actual)
        {
        case UNIDAD:
            unidad++;
            if (unidad > 9)
            {
                unidad = 0;
                carry = true;
                digito_actual = DECENA;
            }
            break;
        case DECENA:
            decena++;
            if (decena > 9)
            {
                decena = 0;
                carry = true;
                digito_actual = CENTENA;
            }
            break;
        case CENTENA:
            centena++;
            if (centena > 9)
            {
                unidad = 0;
                decena = 0;
                centena = 0;
                decima = 0;
                carry = false;
            }
            break;
        default:
            break;
        }

        if (!carry)
        {
            break;
        }

        if (digito_actual > CENTENA)
        {
            break;
        }
    }
}
/* === Public function implementation =============================================================================== */

void contar_decima(void *args)
{
    EventGroupHandle_t _event_group = (EventGroupHandle_t)args;
    TickType_t lastEvent;
    lastEvent = xTaskGetTickCount();

    while (1)
    {
        EventBits_t wBits = xEventGroupWaitBits(_event_group, CUENTA | RESET | EN_PAUSA, pdFALSE, pdFALSE, portMAX_DELAY);
        //ESP_LOGI(TAG, "Estado de los bits en contar_decima: %lu", wBits);
        if ((wBits & CUENTA) != 0)
        {
            //al volver de la pausa, reinicio el lastEvent
            if ((wBits & EN_PAUSA) != 0){ 
                xEventGroupClearBits(_event_group,EN_PAUSA);
                lastEvent = xTaskGetTickCount();

            }
            // ESP_LOGI(TAG, "CUENTA activado - Tick actual: %lu, Last Event: %lu", xTaskGetTickCount(), lastEvent);
            decima = decima + 1;
            xEventGroupSetBits(_event_group, REFRESCO_PANTALLA);
            // ESP_LOGI(TAG, "Bit REFRESCO_PANTALLA activado");
            if (decima == 9)
            {
                decima = 0;
                incrementar_segundo(UNIDAD);
            }
        }
        if ((wBits & RESET) != 0)
        {
            decima = 0;
            unidad = 0;
            decena = 0;
            centena = 0;

            xEventGroupClearBits(_event_group, RESET);
            xEventGroupSetBits(_event_group, REFRESCO_PANTALLA);
        }
        vTaskDelayUntil(&lastEvent, pdMS_TO_TICKS(100));
    }
}

void dibujar_pantalla(void *args)
{
    EventGroupHandle_t _event_group = (EventGroupHandle_t)args;
    ILI9341Init();
    ILI9341Rotate(ILI9341_Landscape_1);

    panel_t segundos = CrearPanel(30, 60, 3, DIGITO_ALTO, DIGITO_ANCHO, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t decimas = CrearPanel(230, 60, 1, DIGITO_ALTO, DIGITO_ANCHO, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);

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
        if (xEventGroupWaitBits(_event_group, REFRESCO_PANTALLA, pdTRUE, pdFALSE, portMAX_DELAY))
        {
            unidad_act = unidad;
            decena_act = decena;
            centena_act = centena;
         //   ESP_LOGI(TAG, "Valores en dibujar_pantalla: %d, %d, %d, %d", unidad_act, decena_act, centena_act, decima);
        }
        if (unidad_act != unidad_ant)
            DibujarDigito(segundos, 2, unidad_act);
        if (decena_act != decena_ant)
            DibujarDigito(segundos, 1, decena_act);
        if (centena_act != centena_ant)
            DibujarDigito(segundos, 0, centena);
        unidad_ant = unidad_act;
        decena_ant = decena_act;
        centena_ant = centena_act;

        ILI9341DrawFilledCircle(220, 150, 5, DIGITO_ENCENDIDO);
        {
            decima_act = decima;
        }
        if (decima_act != decima_ant)
            DibujarDigito(decimas, 0, decima);
        decima_ant = decima_act;
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
    EventGroupHandle_t event_group;
    key_task_t key_args;
    led_task_t leds_args;

    event_group = xEventGroupCreate();

    xEventGroupSetBits(event_group,RED);
    xEventGroupSetBits(event_group,EN_PAUSA);

    if (event_group)
    {
        key_args = malloc(3 * sizeof(key_task_t));
        key_args->event_group = event_group;
        key_args->gpio_id = BOTON1;
        key_args->event_bit = BOTON_ESTADO;
        if (xTaskCreate(tarea_tecla, "boton1", 1024, key_args, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear boton1 ");
    
        key_args = malloc(3 * sizeof(key_task_t));
        key_args->event_group = event_group;
        key_args->gpio_id = BOTON2;
        key_args->event_bit = BOTON_BORRAR;
        if (xTaskCreate(tarea_tecla, "boton2", 1024, key_args, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear boton2");

        key_args = malloc(3 * sizeof(key_task_t));
        key_args->event_group = event_group;
        key_args->gpio_id = BOTON3;
        key_args->event_bit = BOTON_PARCIAL;
        if (xTaskCreate(tarea_tecla, "boton3", 1024, key_args, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear boton3 ");

        leds_args = malloc(5 * sizeof(led_task_t));
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

        if (xTaskCreate(tomar_parcial, "parcial", 2*1024, event_group, tskIDLE_PRIORITY, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear parcial ");

        if (xTaskCreate(contar_decima, "contar", 2 * 1024, event_group, tskIDLE_PRIORITY + 3, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear contar");

        if (xTaskCreate(dibujar_pantalla, "pantalla", 2048, event_group, tskIDLE_PRIORITY + 2, NULL) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear pantalla");
    }
    else
        ESP_LOGE(TAG, "Fallo al crear Eventos ");

}

/* === End of documentation ========================================================================================= */

/** @} End of module definition for doxygen */
