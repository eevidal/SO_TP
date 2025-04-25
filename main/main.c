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
#define BOTON3 GPIO_NUM_14

#define BOTON_ESTADO 1 << 0
#define BOTON_BORRAR 1 << 1
#define BOTON_PARCIAL 1 << 2
#define BLINK 1 << 3
#define RED 1 << 4
#define CUENTA 1 << 5

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
static const char *B2 = "boton_2";

/* === Public variable definitions ================================================================================== */

int decima = 0;
int unidad = 0;
int decena = 0;
int centena = 0;
state_t cuenta = OFF;
state_t luz_verde = OFF;

SemaphoreHandle_t luz_verde_roja_mutex;
SemaphoreHandle_t decima_mutex;
SemaphoreHandle_t cuenta_mutex;

/* === Private function implementation ============================================================================== */

// solo llamar con el mutex de cuenta tomado
void volver_a_cero(void)
{
    unidad = 0;
    decena = 0;
    centena = 0;
    // tomar el mutex de decima
    if (xSemaphoreTake(decima_mutex, portMAX_DELAY) == pdTRUE)
    {
        decima = 0;
        xSemaphoreGive(decima_mutex);
    }
    // soltar el mutex de decima
}

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
                volver_a_cero();
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
        vTaskDelayUntil(&lastEvent, pdMS_TO_TICKS(100));
        // tomar el mutex de decima
        if (xEventGroupWaitBits(_event_group, CUENTA, pdFALSE, pdFALSE, portMAX_DELAY))
        {
            if (xSemaphoreTake(decima_mutex, portMAX_DELAY) == pdTRUE)
            {
                decima = decima + 1;
                if (decima == 9)
                {
                    decima = 0;
                    // soltar el mutex
                    xSemaphoreGive(decima_mutex);
                    incrementar_segundo(UNIDAD);
                }
                else
                {
                    xSemaphoreGive(decima_mutex);
                }
            }
        } // cuenta =OFF
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

    while (1)
    {
        // tomar lock de cuenta
        if (xEventGroupWaitBits(_event_group, CUENTA, pdFALSE, pdFALSE, portMAX_DELAY))
        {
            unidad_act = unidad;
            decena_act = decena;
            centena_act = centena;
        } // soltar lock
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
        if (xSemaphoreTake(decima_mutex, portMAX_DELAY) == pdTRUE)
        {
            decima_act = decima;
            xSemaphoreGive(decima_mutex);
        }
        if (decima_act != decima_ant)
            DibujarDigito(decimas, 0, decima);
        decima_ant = decima_act;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void cambia_estado(void *args)
{
    EventGroupHandle_t _event_group = (EventGroupHandle_t)args;
    //   static state_t estado  = 0;
    while (1)
    {
        EventBits_t wBits = (xEventGroupWaitBits(_event_group, BOTON_ESTADO | CUENTA, pdFALSE, pdFALSE, portMAX_DELAY));
        {

            if ((wBits & CUENTA) && (wBits & BOTON_ESTADO))
            {
                xEventGroupClearBits(_event_group, CUENTA);
                xEventGroupClearBits(_event_group, BOTON_ESTADO);
                xEventGroupClearBits(_event_group, BLINK);
                xEventGroupSetBits(_event_group, RED);
            }
            else if (wBits & BOTON_ESTADO)
            {
                xEventGroupSetBits(_event_group, CUENTA);
                xEventGroupClearBits(_event_group, RED);
                xEventGroupSetBits(_event_group, BLINK);
                xEventGroupClearBits(_event_group, BOTON_ESTADO);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void borrar(void *args)
{
    EventGroupHandle_t _event_group = (EventGroupHandle_t)args;
    while (1)
    {
        EventBits_t wBits = (xEventGroupWaitBits(_event_group, BOTON_BORRAR | CUENTA, pdFALSE, pdFALSE, portMAX_DELAY));
        {
            //   if (xSemaphoreTake(cuenta_mutex, portMAX_DELAY) == pdTRUE)
            {
                if ((wBits & CUENTA))
                {
                    vTaskDelay(pdMS_TO_TICKS(50));
                    continue;
                }
                else if ((wBits & BOTON_BORRAR))
                {
                    xEventGroupClearBits(_event_group, BOTON_BORRAR);
                    volver_a_cero();
                }
                //   xSemaphoreGive(cuenta_mutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void tarea_led(void *args)
{
    led_task_t parametros = (led_task_t)args;
    TickType_t lastEvent;
    gpio_set_direction(parametros->gpio_id_red, GPIO_MODE_OUTPUT);
    gpio_set_direction(parametros->gpio_id_verde, GPIO_MODE_OUTPUT);

    lastEvent = xTaskGetTickCount();

    while (1)
    {
        EventBits_t uxBits = (xEventGroupWaitBits(parametros->event_group, RED | BLINK, pdFALSE, pdFALSE, portMAX_DELAY));
        if ((uxBits & RED) != 0)
        {
            gpio_set_level(parametros->gpio_id_red, 1);
        }
        else
            gpio_set_level(parametros->gpio_id_red, 0);

        if ((uxBits & BLINK) != 0)
        {
            gpio_set_level(parametros->gpio_id_verde, 1);
            vTaskDelayUntil(&lastEvent, pdMS_TO_TICKS(parametros->tiempo));
            gpio_set_level(parametros->gpio_id_verde, 0);
            vTaskDelayUntil(&lastEvent, pdMS_TO_TICKS(parametros->tiempo));
        }
        else
        {
            gpio_set_level(parametros->gpio_id_verde, 0);
        }
    }
}

void app_main(void)
{
    EventGroupHandle_t event_group;
    TaskHandle_t xHandler = NULL;
    key_task_t key_args;
    led_task_t leds_args;
    decima_mutex = xSemaphoreCreateMutex();
    if (decima_mutex == NULL)
        ESP_LOGE(TAG, "Fallo al crear decima_mutex");

    event_group = xEventGroupCreate();

    if (event_group)
    {
        key_args = malloc(sizeof(key_task_t));
        key_args->event_group = event_group;
        key_args->gpio_id = BOTON1;
        key_args->event_bit = BOTON_ESTADO;
        if (xTaskCreate(tarea_tecla, "boton1", 1024, key_args, tskIDLE_PRIORITY, &xHandler) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear boton1 ");
        else
            xHandler = NULL;
        key_args = malloc(sizeof(key_task_t));
        key_args->event_group = event_group;
        key_args->gpio_id = BOTON2;
        key_args->event_bit = BOTON_BORRAR;
        if (xTaskCreate(tarea_tecla, "boton1", 1024, key_args, tskIDLE_PRIORITY, &xHandler) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear boton2");
        else
            xHandler = NULL;
        key_args = malloc(sizeof(key_task_t));
        key_args->event_group = event_group;
        key_args->gpio_id = BOTON3;
        key_args->event_bit = BOTON_PARCIAL;
        if (xTaskCreate(tarea_tecla, "boton3", 1024, key_args, tskIDLE_PRIORITY, &xHandler) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear boton3 ");
        else
            xHandler = NULL;

        leds_args = malloc(sizeof(led_task_t));
        leds_args->event_group = event_group;
        leds_args->event_bit = RED | BLINK;
        leds_args->tiempo = 200;
        leds_args->gpio_id_red = LED_ROJO;
        leds_args->gpio_id_verde = LED_VERDE;

        if (xTaskCreate(tarea_led, "led", 1024, leds_args, tskIDLE_PRIORITY, &xHandler) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear staus ");
        else
            xHandler = NULL;

        if (xTaskCreate(cambia_estado, "status", 1024, event_group, tskIDLE_PRIORITY, &xHandler) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear staus ");
        else
            xHandler = NULL;

        if (xTaskCreate(borrar, "borrar", 1024, event_group, tskIDLE_PRIORITY, &xHandler) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear borrado ");
        else
            xHandler = NULL;

        if (xTaskCreate(contar_decima, "contar", 1024, event_group, tskIDLE_PRIORITY + 3, &xHandler) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear contar");
        else
            xHandler = NULL;

        if (xTaskCreate(dibujar_pantalla, "pantalla", 2048, NULL, tskIDLE_PRIORITY + 3, &xHandler) != pdPASS)
            ESP_LOGE(TAG, "Fallo al crear pantalla");
    }
    else
        ESP_LOGE(TAG, "Fallo al crear Eventos ");

    /*





       if (xTaskCreate(dibujar_pantalla, "pantalla", 2048, NULL, tskIDLE_PRIORITY + 3, &xHandler) != pdPASS)
           ESP_LOGE(TAG, "Fallo al crear pantalla");*/
}

/* === End of documentation ========================================================================================= */

/** @} End of module definition for doxygen */
