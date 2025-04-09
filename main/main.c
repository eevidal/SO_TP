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

/* === Macros definitions =========================================================================================== */

#define DIGITO_ANCHO     60
#define DIGITO_ALTO      100
#define DIGITO_ENCENDIDO ILI9341_RED
#define DIGITO_APAGADO   0x3800
#define DIGITO_FONDO     ILI9341_BLACK

#define LED_ROJO         GPIO_NUM_26 
#define LED_VERDE        GPIO_NUM_27 

#define BOTON1           GPIO_NUM_13
#define BOTON2           GPIO_NUM_32
#define BOTON3           GPIO_NUM_14

/* === Private data type declarations =============================================================================== */

typedef struct blink {
    gpio_num_t led;
    uint16_t tiempo;
} * blink_t;

typedef enum { UNIDAD = 0, DECENA = 1, CENTENA = 2 } digito_t;

typedef enum {
    OFF = 0,
    ON = 1,
} state_t;

/* === Private variable declarations ================================================================================ */

static const struct blink parametros[] = {
    {.led = LED_VERDE, .tiempo = 300},
    {.led = LED_ROJO, .tiempo = 100},
};


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
void volver_a_cero(void) {

    unidad = 0;
    decena = 0;
    centena = 0;
    // tomar el mutex de decima
    if (xSemaphoreTake(decima_mutex, portMAX_DELAY) == pdTRUE) {
        decima = 0;
        xSemaphoreGive(decima_mutex);
    }
    // soltar el mutex de decima
}

void incrementar_segundo(digito_t digito_inicial) {
    if (xSemaphoreTake(cuenta_mutex, portMAX_DELAY) == pdTRUE) {
        digito_t digito_actual = digito_inicial;
        bool carry = true;
        while (carry) {
            carry = false;
            switch (digito_actual) {
            case UNIDAD:
                unidad++;
                if (unidad > 9) {
                    unidad = 0;
                    carry = true;
                    digito_actual = DECENA;
                }
                break;
            case DECENA:
                decena++;
                if (decena > 9) {
                    decena = 0;
                    carry = true;
                    digito_actual = CENTENA;
                }
                break;
            case CENTENA:
                centena++;
                if (centena > 9) {
                    volver_a_cero();
                    carry = false;
                }
                break;
            default:
                break;
            }

            if (!carry) {
                break;
            }

            if (digito_actual > CENTENA) {
                break;
            }
        }
        xSemaphoreGive(cuenta_mutex);
    }
}
/* === Public function implementation =============================================================================== */

void contar_decima(void * args) {
    TickType_t lastEvent;
    lastEvent = xTaskGetTickCount();
    while (1) {
        vTaskDelayUntil(&lastEvent, pdMS_TO_TICKS(100));
        // tomar el mutex de decima
        if (xSemaphoreTake(cuenta_mutex, portMAX_DELAY) == pdTRUE) {
            if (cuenta == ON) {
                if (xSemaphoreTake(decima_mutex, portMAX_DELAY) == pdTRUE) {
                    decima = decima + 1;
                    if (decima == 9) {
                        decima = 0;
                        // soltar el mutex
                        xSemaphoreGive(decima_mutex);
                        xSemaphoreGive(cuenta_mutex);
                        incrementar_segundo(UNIDAD);
                    } else {
                        xSemaphoreGive(decima_mutex);
                    }
                } else {
                    // ESP_LOGI("CONTAR", "falle en tomar el lock decima\n");
                }
            } // cuenta =OFF
            xSemaphoreGive(cuenta_mutex);
        } else {

            //  ESP_LOGI("CONTAR", "falle en tomar el lock\n");
        }
        //   ESP_LOGI("CONTAR", "Decima: %d", decima);
    }
}
void blinking(void * args) {
    blink_t parametros = (blink_t)args;
    TickType_t lastEvent;
    gpio_set_direction(parametros->led, GPIO_MODE_OUTPUT);
    gpio_set_level(parametros->led, 0);
    lastEvent = xTaskGetTickCount();

    while (1) {

        if (xSemaphoreTake(luz_verde_roja_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            if (luz_verde == ON) {
                gpio_set_level(parametros->led, 1);
                vTaskDelayUntil(&lastEvent, pdMS_TO_TICKS(parametros->tiempo));
                gpio_set_level(parametros->led, 0);
                xSemaphoreGive(luz_verde_roja_mutex);
                vTaskDelayUntil(&lastEvent, pdMS_TO_TICKS(parametros->tiempo));
            } else {

                gpio_set_level(parametros->led, 0); // Ensure LED is off when not blinking
                xSemaphoreGive(luz_verde_roja_mutex);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
    }
}

void red(void * args) {
    blink_t parametros = (blink_t)args;

    gpio_set_direction(LED_ROJO, GPIO_MODE_OUTPUT);
    gpio_set_level(parametros->led, 1);
    while (1) {
        // tomar lock luz verde roja
        if (xSemaphoreTake(luz_verde_roja_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            if (luz_verde == ON)

                gpio_set_level(parametros->led, 0);
            else
                gpio_set_level(parametros->led, 1);
            xSemaphoreGive(luz_verde_roja_mutex);
            // soltar lock luz verde roja
        }
        //  vTaskDelay();
    }
}

void dibujar_pantalla(void * args) {
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

    while (1) {
        // tomar lock de cuenta
        if (xSemaphoreTake(cuenta_mutex, portMAX_DELAY) == pdTRUE) {
            unidad_act = unidad;
            decena_act = decena;
            centena_act = centena;
            xSemaphoreGive(cuenta_mutex);
        }//soltar lock
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
        if (xSemaphoreTake(decima_mutex, portMAX_DELAY) == pdTRUE) {
            decima_act = decima;
            xSemaphoreGive(decima_mutex);
        }
        if (decima_act != decima_ant)
            DibujarDigito(decimas, 0, decima);
        decima_ant = decima_act;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void leer_boton1(void * args) {
    gpio_set_direction(BOTON1, GPIO_MODE_INPUT);
    gpio_pullup_en(BOTON1);
    int level;
    level = gpio_get_level(BOTON1);

    while (1) {
        while (level == 1) {
            level = gpio_get_level(BOTON1);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        if (level == 0) { 
            // ESP_LOGI("BOTON1", "presionado");
            if (xSemaphoreTake(cuenta_mutex, portMAX_DELAY) == pdTRUE) {
                if (cuenta == ON) {
                    cuenta = OFF;
                    if (xSemaphoreTake(luz_verde_roja_mutex, portMAX_DELAY) == pdTRUE) {
                        luz_verde = OFF;
                        xSemaphoreGive(luz_verde_roja_mutex);
                    }
                } else {
                    cuenta = ON;
                    if (xSemaphoreTake(luz_verde_roja_mutex, portMAX_DELAY) == pdTRUE) {
                        luz_verde = ON;
                        xSemaphoreGive(luz_verde_roja_mutex);
                    }
                }
                xSemaphoreGive(cuenta_mutex);
            }

            vTaskDelay(pdMS_TO_TICKS(20));
        }
        while (level == 0) {
            level = gpio_get_level(BOTON1);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        // ESP_LOGI("BOTON1", "no presionado");
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void leer_boton2(void * args) {
    gpio_set_direction(BOTON2, GPIO_MODE_INPUT);
    gpio_pullup_en(BOTON2);
    int level;
    level = gpio_get_level(BOTON2);
    
    while (1) {
        while (level == 1) {
            level = gpio_get_level(BOTON2);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        if (level == 0) {
            if (xSemaphoreTake(cuenta_mutex, portMAX_DELAY) == pdTRUE) {
                if (cuenta == OFF) {
                    volver_a_cero();
                }
                xSemaphoreGive(cuenta_mutex);
                ESP_LOGI("BOTON2", "presionado");
            }
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        while (level == 0) {
            level = gpio_get_level(BOTON2);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        ESP_LOGI("BOTON2", "solto");
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void) {
    cuenta_mutex = xSemaphoreCreateMutex();
    decima_mutex = xSemaphoreCreateMutex();
    luz_verde_roja_mutex = xSemaphoreCreateMutex();
    xTaskCreate(blinking, "Verde", 1024, (void *)&parametros[0], tskIDLE_PRIORITY, NULL);
    xTaskCreate(red, "roja", 1024, (void *)&parametros[1], tskIDLE_PRIORITY, NULL);
    xTaskCreate(leer_boton1, "boton1", 1024, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(leer_boton2, "boton2", 2 * 1024, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(contar_decima, "contar", 1024, NULL, tskIDLE_PRIORITY + 3, NULL);
    xTaskCreate(dibujar_pantalla, "pantalla", 2048, NULL, tskIDLE_PRIORITY + 3, NULL);
}

/* === End of documentation ========================================================================================= */

/** @} End of module definition for doxygen */

