#ifndef _DISPLAY_H
#define _DISPLAY_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ili9341.h"
#include "digitos.h"

#define DIGITO_ANCHO 30
#define DIGITO_ALTO 60
#define DIGITO_ANCHO_P 30
#define DIGITO_ALTO_P 50
#define DIGITO_ENCENDIDO ILI9341_WHITE
#define DIGITO_APAGADO 0x3800
#define DIGITO_FONDO ILI9341_BLACK

#define DIBUJAR_PARCIAL(panel_base, centena, decena, unidad, decima) \
    DibujarDigito(panel_base, 2, unidad);                            \
    DibujarDigito(panel_base, 1, decena);                            \
    DibujarDigito(panel_base, 0, centena);                           \
    DibujarDigito(panel_base##_d, 0, decima);

#define DIBUJAR_SI_CAMBIA(actual, anterior, panel, posicion) \
    if (actual != anterior)                                  \
    {                                                        \
        DibujarDigito(panel, posicion, actual);              \
    }

#define DIBUJAR_YEAR(panel_base, mil, centena, decena, unidad) \
    DibujarDigito(panel_base, 3, unidad);                      \
    DibujarDigito(panel_base, 2, decena);                      \
    DibujarDigito(panel_base, 1, centena);                     \
    DibujarDigito(panel_base, 0, mil);

#define DIBUJAR_T(panel_base, decena, unidad) \
    DibujarDigito(panel_base, 1, unidad);     \
    DibujarDigito(panel_base, 0, decena);

typedef struct display_task
{
    QueueHandle_t qcrono;
    QueueHandle_t qclock, qalarm, qconf;
    EventGroupHandle_t event_group;
    uint8_t parcial_bits;
    uint8_t reset_bits;
    bool alarm_set;
    int selected;


} display_task;

typedef struct display_task *display_task_t;

void dibujar_pantalla(void *args);

#endif