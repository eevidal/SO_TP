#ifndef _DISPLAY_H
#define _DISPLAY_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ili9341.h"
#include "digitos.h"

#define DIGITO_ANCHO 55
#define DIGITO_ALTO 90
#define DIGITO_ANCHO_A 30
#define DIGITO_ALTO_A 60
#define DIGITO_ANCHO_P 30
#define DIGITO_ALTO_P 50
#define DIGITO_ANCHO_E 15
#define DIGITO_ALTO_E 30
#define DIGITO_ENCENDIDO ILI9341_WHITE
#define DIGITO_ENCENDIDO_G ILI9341_GREEN
#define DIGITO_ENCENDIDO_R ILI9341_RED
#define DIGITO_ENCENDIDO_B ILI9341_BLUE
#define DIGITO_ENCENDIDO_Y ILI9341_YELLOW
#define DIGITO_ENCENDIDO_DG ILI9341_DARKGREY

#define DIGITO_APAGADO 0x3800
#define DIGITO_FONDO ILI9341_BLACK

#define DIBUJAR_SI_CAMBIA(actual, anterior, panel, posicion) \
    if (actual != anterior)                                  \
    {                                                        \
        DibujarDigito(panel, posicion, actual);              \
    }

#define EXTRAER_MILES(n) ((n) / 1000)
#define RESTO_MILES(n) ((n) % 1000)
#define EXTRAER_CENTENAS(n) ((n) / 100)
#define RESTO_CENTENAS(n) ((n) % 100)
#define EXTRAER_DECENAS(n) ((n) / 10)
#define EXTRAER_UNIDADES(n) ((n) % 10)

#define DIBUJAR_YEAR(panel_base, year_ac, year_ant)                            \
    {                                                                          \
        int miles = EXTRAER_MILES(year_ac);                                    \
        int centenas = EXTRAER_CENTENAS(RESTO_MILES(year_ac));                 \
        int decenas = EXTRAER_DECENAS(RESTO_CENTENAS(RESTO_MILES(year_ac)));   \
        int unidades = EXTRAER_UNIDADES(RESTO_CENTENAS(RESTO_MILES(year_ac))); \
        _DIBUJAR_YEAR(panel_base, miles, centenas, decenas, unidades);         \
    }

#define _DIBUJAR_YEAR(panel_base, mil, centena, decena, unidad) \
    DibujarDigito(panel_base, 3, unidad);                       \
    DibujarDigito(panel_base, 2, decena);                       \
    DibujarDigito(panel_base, 1, centena);                      \
    DibujarDigito(panel_base, 0, mil);

#define DIBUJAR_HORA(panel_base, hora_ac, hora_ant) \
    {                                               \
        DIBUJAR_T(panel_base, hora_ac / 10, hora_ac % 10)}

/* Podría mos cambiarla para mostrar las 3 primeras letras del mes*/
#define DIBUJAR_MES(panel_base, mes_ac, mes_ant)        \
    {                                                   \
        DIBUJAR_T(panel_base, mes_ac / 10, mes_ac % 10) \
    }

#define DIBUJAR_T(panel_base, decena, unidad) \
    DibujarDigito(panel_base, 1, unidad);     \
    DibujarDigito(panel_base, 0, decena);

#define DIBUJAR_TODO_RELOJ(_clock_act, _clock_ant, h, m, s, d, mes, a) \
    do                                                                 \
    {                                                                  \
        DIBUJAR_HORA(h, _clock_act.hr , _clock_ant.hr );          \
        DIBUJAR_HORA(m, _clock_act.min, _clock_ant.min);               \
        DIBUJAR_HORA(s, _clock_act.sec, _clock_ant.sec);               \
        DIBUJAR_HORA(d, _clock_act.day, _clock_ant.day);               \
        DIBUJAR_MES(mes, _clock_act.month, _clock_ant.month);          \
        DIBUJAR_YEAR(a, _clock_act.year, _clock_ant.year);             \
    } while (0)


    #define DIBUJAR_TODO_RELOJ_A(_clock_act, _clock_ant, h, m, s, d, mes, a) \
    do                                                                 \
    {                                                                  \
        DIBUJAR_HORA(h, _clock_act->hr , _clock_ant->hr);          \
        DIBUJAR_HORA(m, _clock_act->min, _clock_ant->min);               \
        DIBUJAR_HORA(s, _clock_act->sec, _clock_ant->sec);               \
        DIBUJAR_HORA(d, _clock_act->day, _clock_ant->day);               \
        DIBUJAR_MES(mes, _clock_act->month, _clock_ant->month);          \
        DIBUJAR_YEAR(a, _clock_act->year, _clock_ant->year);             \
    } while (0)

#define MASCARA_A_POSICION(mascara) ( (mascara == 1) ? 0 : \
                                                 (mascara == 2) ? 1 : \
                                                 (mascara == 4) ? 2 : \
                                                 (mascara == 8) ? 3 : \
                                                 (mascara == 16) ? 4 : \
                                                 (mascara == 32) ? 5 : \
                                                 (mascara == 64) ? 6 : \
                                                 (mascara == 128) ? 7 : -1 ) 

typedef struct display_task
{
    QueueHandle_t qcrono;
    QueueHandle_t qclock, qalarm, qconf;
    EventGroupHandle_t event_group;
    uint8_t parcial_bits;
    uint32_t reset_bits;
    int selected;

} display_task;

typedef struct display_task *display_task_t;

void dibujar_pantalla(void *args);

#endif