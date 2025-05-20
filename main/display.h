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

#define CRONO_RESET_PANTALLA()                                \
    do                                                          \
    {   ILI9341Fill(DIGITO_APAGADO);                            \
        DibujarDigito(segundos, 2, 0);                          \
        DibujarDigito(segundos, 1, 0);                          \
        DibujarDigito(segundos, 0, 0);                          \
        DibujarDigito(decimas, 0, 0);                           \
        ILI9341DrawFilledCircle(178, 95, 5, DIGITO_ENCENDIDO);  \
        ILI9341DrawFilledCircle(113, 160, 5, DIGITO_ENCENDIDO); \
        ILI9341DrawFilledCircle(145, 220, 5, DIGITO_ENCENDIDO); \
        ILI9341DrawFilledCircle(178, 280, 5, DIGITO_ENCENDIDO); \
        DIBUJAR_PARCIAL(parcial1, 0, 0, 0, 0);                  \
        DIBUJAR_PARCIAL(parcial2, 0, 0, 0, 0);                  \
        DIBUJAR_PARCIAL(parcial3, 0, 0, 0, 0);                  \
    } while (0)



#define CLOCK_RESET_PANTALLA_0(_clock)                 \
    do                                                         \
    {                                                          \
        ILI9341Fill(DIGITO_APAGADO);                           \
        DIBUJAR_HORA(rhoras, 0, 0);                          \
        DIBUJAR_HORA(rminutos, 0, 0);                          \
        DIBUJAR_HORA(rsegundos, 0, 0);                         \
        DIBUJAR_HORA(rdia, 0, 0);                              \
        DIBUJAR_MES(rmes, 0, 0);                               \
        DIBUJAR_YEAR(ryear, 0, 0);                             \
        ILI9341DrawFilledCircle(70, 55, 3, DIGITO_ENCENDIDO);  \
        ILI9341DrawFilledCircle(70, 35, 3, DIGITO_ENCENDIDO);  \
        ILI9341DrawFilledCircle(145, 55, 3, DIGITO_ENCENDIDO); \
        ILI9341DrawFilledCircle(145, 35, 3, DIGITO_ENCENDIDO); \
    } while (0);

#define CLOCK_RESET_PANTALLA()                                 \
    do                                                         \
    {                                                          \
        ILI9341Fill(DIGITO_APAGADO);                           \
        ILI9341DrawFilledCircle(75, 55, 3, DIGITO_ENCENDIDO);  \
        ILI9341DrawFilledCircle(75, 35, 3, DIGITO_ENCENDIDO);  \
        ILI9341DrawFilledCircle(150, 55, 3, DIGITO_ENCENDIDO); \
        ILI9341DrawFilledCircle(150, 35, 3, DIGITO_ENCENDIDO); \
    } while (0);

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

#define EXTRAER_MILES(n) ((n) / 1000)
#define RESTO_MILES(n) ((n) % 1000)
#define EXTRAER_CENTENAS(n) ((n) / 100)
#define RESTO_CENTENAS(n) ((n) % 100)
#define EXTRAER_DECENAS(n) ((n) / 10)
#define EXTRAER_UNIDADES(n) ((n) % 10)

#define DIBUJAR_YEAR(panel_base, year_ac, year_ant)                            \
    if (year_ac != year_ant)                                                   \
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

#define DIBUJAR_HORA(panel_base, hora_ac, hora_ant)       \
    if (hora_ac != hora_ant)                              \
    {                                                     \
        DIBUJAR_T(panel_base, hora_ac / 10, hora_ac % 10) \
    }

/* Podría mos cambiarla para mostrar las 3 primeras letras del mes*/
#define DIBUJAR_MES(panel_base, mes_ac, mes_ant)        \
    if (mes_ac != mes_ant)                              \
    {                                                   \
        DIBUJAR_T(panel_base, mes_ac / 10, mes_ac % 10) \
    }

#define DIBUJAR_T(panel_base, decena, unidad) \
    DibujarDigito(panel_base, 1, unidad);     \
    DibujarDigito(panel_base, 0, decena);

#define DIBUJAR_TODO_RELOJ(_clock_act, _clock_ant_act)               \
    do                                                               \
    {                                                                \
        DIBUJAR_T(rhoras, _clock_act.hr/10, _clock_ant_act.hr%10);      \
        DIBUJAR_HORA(rminutos, _clock_act.min, _clock_ant_act.min);  \
        DIBUJAR_HORA(rsegundos, _clock_act.sec, _clock_ant_act.sec); \
        DIBUJAR_HORA(rdia, _clock_act.day, _clock_ant_act.day);      \
        DIBUJAR_MES(rmes, _clock_act.month, _clock_ant_act.month);   \
        DIBUJAR_YEAR(ryear, _clock_act.year, _clock_ant_act.year);   \
    } while (0)

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