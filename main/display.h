#ifndef _DISPLAY_H
#define _DISPLAY_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ili9341.h"
#include "digitos.h"

/**
 * @name Digit Display Dimensions
 * @brief Constants defining the dimensions for various digit types on the display.
 * @{
 */
#define DIGITO_ANCHO 55   /**< Width for large digits. */
#define DIGITO_ALTO 90    /**< Height for large digits. */
#define DIGITO_ANCHO_A 30 /**< Width for alarm/clock digits. */
#define DIGITO_ALTO_A 60  /**< Height for alarm/clock digits. */
#define DIGITO_ANCHO_P 30 /**< Width for partial/small digits. */
#define DIGITO_ALTO_P 50  /**< Height for partial/small digits. */
#define DIGITO_ANCHO_E 15 /**< Width for mode indicator digits. */
#define DIGITO_ALTO_E 30  /**< Height for mode indicator digits. */
/** @} */

/**
 * @name Digit Display Colors
 * @brief Constants defining the colors for various display elements.
 * @{
 */
#define DIGITO_ENCENDIDO ILI9341_WHITE       /**< Default color for active segments of digits. */
#define DIGITO_ENCENDIDO_G ILI9341_GREEN     /**< Green color for active segments. */
#define DIGITO_ENCENDIDO_R ILI9341_RED       /**< Red color for active segments. */
#define DIGITO_ENCENDIDO_B ILI9341_BLUE      /**< Blue color for active segments. */
#define DIGITO_ENCENDIDO_Y ILI9341_YELLOW    /**< Yellow color for active segments. */
#define DIGITO_ENCENDIDO_DG ILI9341_DARKGREY /**< Dark grey color for active segments. */
#define DIGITO_APAGADO 0x3800                /**< Color for inactive segments of digits (a shade of dark green/brown). */
#define DIGITO_FONDO ILI9341_BLACK           /**< Background color of the display. */
/** @} */

/**
 * @brief Macro to draw a digit only if its value has changed.
 * @param actual The current digit value.
 * @param anterior The previous digit value.
 * @param panel The display panel structure.
 * @param posicion The position within the panel to draw the digit.
 */
#define DIBUJAR_SI_CAMBIA(actual, anterior, panel, posicion) \
    if (actual != anterior)                                  \
    {                                                        \
        DibujarDigito(panel, posicion, actual);              \
    }

/**
 * @name Number Extraction Macros
 * @brief Macros for extracting individual digits (thousands, hundreds, tens, units) from a number.
 * @{
 */
#define EXTRAER_MILES(n) ((n) / 1000)   /**< Extracts the thousands digit. */
#define RESTO_MILES(n) ((n) % 1000)     /**< Gets the remainder after extracting thousands. */
#define EXTRAER_CENTENAS(n) ((n) / 100) /**< Extracts the hundreds digit. */
#define RESTO_CENTENAS(n) ((n) % 100)   /**< Gets the remainder after extracting hundreds. */
#define EXTRAER_DECENAS(n) ((n) / 10)   /**< Extracts the tens digit. */
#define EXTRAER_UNIDADES(n) ((n) % 10)  /**< Extracts the units digit. */
/** @} */

/**
 * @brief Macro to draw a 4-digit year on a display panel.
 * @param panel_base The base panel for drawing digits.
 * @param year_ac The current year value.
 * @param year_ant The previous year value (not directly used by this macro, but kept for consistency).
 */
#define DIBUJAR_YEAR(panel_base, year_ac, year_ant)                            \
    {                                                                          \
        int miles = EXTRAER_MILES(year_ac);                                    \
        int centenas = EXTRAER_CENTENAS(RESTO_MILES(year_ac));                 \
        int decenas = EXTRAER_DECENAS(RESTO_CENTENAS(RESTO_MILES(year_ac)));   \
        int unidades = EXTRAER_UNIDADES(RESTO_CENTENAS(RESTO_MILES(year_ac))); \
        _DIBUJAR_YEAR(panel_base, miles, centenas, decenas, unidades);         \
    }

/**
 * @brief Internal macro to draw individual year digits.
 * @param panel_base The base panel for drawing digits.
 * @param mil The thousands digit.
 * @param centena The hundreds digit.
 * @param decena The tens digit.
 * @param unidad The units digit.
 */
#define _DIBUJAR_YEAR(panel_base, mil, centena, decena, unidad) \
    DibujarDigito(panel_base, 3, unidad);                       \
    DibujarDigito(panel_base, 2, decena);                       \
    DibujarDigito(panel_base, 1, centena);                      \
    DibujarDigito(panel_base, 0, mil);

/**
 * @brief Macro to draw a 2-digit hour on a display panel.
 * @param panel_base The base panel for drawing digits.
 * @param hora_ac The current hour value.
 * @param hora_ant The previous hour value (not directly used by this macro, but kept for consistency).
 */
#define DIBUJAR_HORA(panel_base, hora_ac, hora_ant) \
    {                                               \
        DIBUJAR_T(panel_base, hora_ac / 10, hora_ac % 10)}

#define DIBUJAR_HORA_B(panel_base, hora_ac, hora_ant)     \
    for (int i = 0; i < 1; ++i)                           \
    {                                                     \
        BorrarDigito(panel_base, 1);                      \
        BorrarDigito(panel_base, 0);                      \
        vTaskDelay(pdMS_TO_TICKS(100));                   \
        DIBUJAR_T(panel_base, hora_ac / 10, hora_ac % 10) \
    }

/**
 * @brief Macro to draw a 2-digit month on a display panel.
 * @param panel_base The base panel for drawing digits.
 * @param mes_ac The current month value.
 * @param mes_ant The previous month value
 **/

// Podría mos cambiarla para mostrar las 3 primeras letras del mes
#define DIBUJAR_MES(panel_base, mes_ac, mes_ant) \
    {                                            \
        DIBUJAR_T(panel_base, mes_ac / 10, mes_ac % 10)}

/**
 * @brief Macro to draw a 2-digit month with a blinking effect.
 * @param panel_base The base panel for drawing digits.
 * @param mes_ac The current month value.
 * @param mes_ant The previous month value
 */
#define DIBUJAR_MES_B(panel_base, mes_ac, mes_ant)      \
    for (int i = 0; i < 1; ++i)                         \
    {                                                   \
        BorrarDigito(panel_base, 1);                    \
        BorrarDigito(panel_base, 0);                    \
        vTaskDelay(pdMS_TO_TICKS(100));                 \
        DIBUJAR_T(panel_base, mes_ac / 10, mes_ac % 10) \
    }

/**
 * @brief Internal macro to draw a 2-digit number (tens and units).
 * @param panel_base The base panel for drawing digits.
 * @param decena The tens digit.
 * @param unidad The units digit.
 */
#define DIBUJAR_T(panel_base, decena, unidad) \
    DibujarDigito(panel_base, 1, unidad);     \
    DibujarDigito(panel_base, 0, decena);

/**
 * @brief Macro to draw the entire clock display.
 * @param _clock_act The current clock time (`time_clock` struct).
 * @param _clock_ant The previous clock time (`time_clock` struct).
 * @param h Panel for hours.
 * @param m Panel for minutes.
 * @param s Panel for seconds.
 * @param d Panel for day.
 * @param mes Panel for month.
 * @param a Panel for year.
 */
#define DIBUJAR_TODO_RELOJ(_clock_act, _clock_ant, h, m, s, d, mes, a) \
    do                                                                 \
    {                                                                  \
        DIBUJAR_HORA(h, _clock_act.hr, _clock_ant.hr);                 \
        DIBUJAR_HORA(m, _clock_act.min, _clock_ant.min);               \
        DIBUJAR_HORA(s, _clock_act.sec, _clock_ant.sec);               \
        DIBUJAR_HORA(d, _clock_act.day, _clock_ant.day);               \
        DIBUJAR_MES(mes, _clock_act.month, _clock_ant.month);          \
        DIBUJAR_YEAR(a, _clock_act.year, _clock_ant.year);             \
    } while (0)

    /**
 * @brief Macro to draw the entire clock display with a selected blinking segment.
 * @param _clock_act The current clock time (`time_clock` struct).
 * @param _clock_ant The previous clock time (`time_clock` struct).
 * @param h Panel for hours.
 * @param m Panel for minutes.
 * @param s Panel for seconds.
 * @param d Panel for day.
 * @param mes Panel for month.
 * @param a Panel for year.
 * @param sel The index of the segment to blink (0 for hours, 1 for minutes, etc.).
 */
#define DIBUJAR_TODO_RELOJ_B(_clock_act, _clock_ant, h, m, s, d, mes, a, sel) \
    do                                                                        \
    {                                                                         \
        if (sel == 0)                                                         \
            DIBUJAR_HORA_B(h, _clock_act.hr, _clock_ant.hr)                   \
        else                                                                  \
            DIBUJAR_HORA(h, _clock_act.hr, _clock_ant.hr);                    \
        if (sel == 1)                                                         \
            DIBUJAR_HORA_B(m, _clock_act.min, _clock_ant.min)                 \
        else                                                                  \
            DIBUJAR_HORA(m, _clock_act.min, _clock_ant.min);                  \
        if (sel == 2)                                                         \
            DIBUJAR_HORA_B(s, _clock_act.sec, _clock_ant.sec)                 \
        else                                                                  \
            DIBUJAR_HORA(s, _clock_act.sec, _clock_ant.sec);                  \
        if (sel == 3)                                                         \
            DIBUJAR_HORA_B(d, _clock_act.day, _clock_ant.day)                 \
        else                                                                  \
            DIBUJAR_HORA(d, _clock_act.day, _clock_ant.day);                  \
        if (sel == 4)                                                         \
            DIBUJAR_MES(mes, _clock_act.month, _clock_ant.month)              \
        else                                                                  \
            DIBUJAR_MES(mes, _clock_act.month, _clock_ant.month);             \
        DIBUJAR_YEAR(a, _clock_act.year, _clock_ant.year);                    \
    } while (0)

    /**
 * @brief Macro to draw the entire clock display for alarm configuration with a selected blinking segment.
 * @param _clock_act Pointer to the current alarm time (`time_clock` struct).
 * @param _clock_ant Pointer to the previous alarm time (`time_clock` struct).
 * @param h Panel for hours.
 * @param m Panel for minutes.
 * @param s Panel for seconds.
 * @param d Panel for day.
 * @param mes Panel for month.
 * @param a Panel for year.
 * @param sel The index of the segment to blink (0 for hours, 1 for minutes, etc.).
 */
#define DIBUJAR_TODO_RELOJ_A(_clock_act, _clock_ant, h, m, s, d, mes, a, sel) \
    do                                                                        \
    {                                                                         \
        if (sel == 0)                                                         \
            DIBUJAR_HORA_B(h, _clock_act->hr, _clock_ant->hr)                 \
        else                                                                  \
            DIBUJAR_HORA(h, _clock_act->hr, _clock_ant->hr);                  \
        if (sel == 1)                                                         \
            DIBUJAR_HORA_B(m, _clock_act->min, _clock_ant->min)               \
        else                                                                  \
            DIBUJAR_HORA(m, _clock_act->min, _clock_ant->min)                 \
        if (sel == 2)                                                         \
            DIBUJAR_HORA_B(s, _clock_act->sec, _clock_ant->sec)               \
        else                                                                  \
            DIBUJAR_HORA(s, _clock_act->sec, _clock_ant->sec)                 \
        if (sel == 3)                                                         \
            DIBUJAR_HORA_B(d, _clock_act->day, _clock_ant->day)               \
        else                                                                  \
            DIBUJAR_HORA(d, _clock_act->day, _clock_ant->day);                \
        if (sel == 4)                                                         \
            DIBUJAR_MES_B(mes, _clock_act->month, _clock_ant->month)          \
        else                                                                  \
            DIBUJAR_MES(mes, _clock_act->month, _clock_ant->month);           \
        DIBUJAR_YEAR(a, _clock_act->year, _clock_ant->year);                  \
    } while (0)


/**
 * @brief Converts a bitmask (power of 2) to its corresponding position (0-indexed).
 * @param mascara The bitmask (e.g., 1, 2, 4, 8, 16, etc.).
 * @return The 0-indexed position, or -1 if the mask is not a power of 2 within the defined range.
 */
#define MASCARA_A_POSICION(mascara) ((mascara == 1) ? 0 : (mascara == 2) ? 1 \
                                                      : (mascara == 4)   ? 2 \
                                                      : (mascara == 8)   ? 3 \
                                                      : (mascara == 16)  ? 4 \
                                                      : (mascara == 32)  ? 5 \
                                                      : (mascara == 64)  ? 6 \
                                                      : (mascara == 128) ? 7 \
                                                                         : -1)


/**
 * @brief Structure to hold parameters for the display task.
 */
typedef struct display_task
{
    QueueHandle_t qcrono;        /**< Queue handle for stopwatch time data. */
    QueueHandle_t qclock;        /**< Queue handle for current clock time data. */
    QueueHandle_t qalarm;        /**< Queue handle for alarm time configuration data. */
    QueueHandle_t qconf;         /**< Queue handle for clock configuration data (e.g., selected segment). */
    EventGroupHandle_t event_group; /**< Event group for mode changes and display specific events. */
    uint8_t parcial_bits;      /**< Event bitmask for triggering partial (lap) time display. */
    uint32_t reset_bits;       /**< Event bitmask for triggering a display reset. */
    int selected;              /**< Initial selected item for display configuration. */
} display_task;

/**
 * @brief Pointer to the display_task structure.
 */
typedef struct display_task *display_task_t;

void dibujar_pantalla(void *args);

#endif