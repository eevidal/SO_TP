#ifndef _TIME_SRTUCT_H_
#define _TIME_SRTUCT_H_

/**
 * @brief Enumeration for individual digits within a number.
 * Used primarily for stopwatch time manipulation.
 */
typedef enum
{
   UNIDAD = 0,  /**< Represents the units digit. */
    DECENA = 1,  /**< Represents the tens digit. */
    CENTENA = 2  /**< Represents the hundreds digit. */
} digito_t;

/**
 * @brief Enumeration for the operational state of the stopwatch.
 */
typedef enum
{
    STOP = 0,      /**< Stopwatch is stopped. */
    CORRIENDO = 1, /**< Stopwatch is running. */
    PAUSA = 2      /**< Stopwatch is paused. */
} crono_estado_t;

// tiempo en segundos
/**
 * @brief Structure to represent a stopwatch time.
 * This structure holds time in terms of tenths of seconds, units, tens, and hundreds of seconds.
 */
typedef struct time_struct
{
    int decima;  /**< Tenths of a second (0-9). */
    int unidad;  /**< Units of seconds (0-9). */
    int decena;  /**< Tens of seconds (0-9). */
    int centena; /**< Hundreds of seconds (0-9). */
} time_struct;


/**
 * @brief Pointer to a `time_struct`.
 */
typedef struct time_struct *time_struct_t;

/**
 * @brief Structure to represent a date and time.
 * Used for the main clock and alarm functionality.
 */
typedef  struct time_clock
    {
        int year;
        int month;
        int day;
        int hr;
        int min;
        int sec;
} time_clock;


typedef struct time_clock *time_clock_t;

/**
 * @brief Structure to hold alarm settings.
 * Includes a pointer to the alarm time, currently selected field for configuration,
 * and a flag indicating if the alarm is set.
 */
typedef struct clock_settings{
    time_clock_t t;
    int select;
    bool set;
} clock_settings; 

/**
 * @brief Pointer to a `clock_settings` for convenience.
 */
typedef struct clock_settings *clock_settings_t;



/**
 * @brief Sets all fields of a `time_struct` (stopwatch) to zero.
 * @param timer Pointer to the `time_struct` to be reset.
 */
void time_cero(time_struct_t timer);

/**
 * @brief Increments the `time_struct` by one "tick" (0.1 seconds).
 * Handles rollovers for tenths, units, tens, and hundreds of seconds.
 * @param timer Pointer to the `time_struct` to be incremented.
 */
void time_tick(time_struct_t timer);

/**
 * @brief Increments the seconds part of a `time_struct`, handling carries to tens and hundreds.
 * This is an internal helper function used by `time_tick`.
 * @param timer Pointer to the `time_struct` to be incremented.
 * @param digito_inicial The digit from which to start the increment (e.g., UNIDAD for seconds).
 */
void time_incrementar_segundo(time_struct_t timer, digito_t digito_inicial);

/**
 * @brief Initializes a `time_clock` structure to a default start time (e.g., 2025/01/01 00:00:00).
 * @param timer Pointer to the `time_clock` to be initialized.
 */
void clock_init(time_clock_t timer);

/**
 * @brief Initializes a `time_clock` structure for an alarm to a default value (e.g., 2025/01/01 00:00:10).
 * @param timer Pointer to the `time_clock` (alarm) to be initialized.
 */
void clock_alarm_init(time_clock_t timer);


/**
 * @brief Increments the `time_clock` by one second.
 * Handles rollovers for seconds, minutes, hours, days, months, and years.
 * @param timer Pointer to the `time_clock` to be incremented.
 */
void clock_tick(time_clock_t timer);

/**
 * @brief Increments the minutes part of a `time_clock` structure, handling rollover to hours.
 * @param timer Pointer to the `time_clock` to be incremented.
 */
void clock_incrementar_minuto(time_clock_t timer);

/**
 * @brief Increments the `min` field of a `time_clock_t` by a specified amount.
 * @param timer Pointer to the `time_clock_t` to modify.
 * @param min The number of minutes to increment by.
 */
void clock_incrementar_hora(time_clock_t timer);

/**
 * @brief Increments the days part of a `time_clock` structure, handling rollover to months
 * and considering the number of days in the current month/year.
 * @param timer Pointer to the `time_clock` to be incremented.
 */
void clock_incrementar_dia(time_clock_t timer);

/**
 * @brief Increments the months part of a `time_clock` structure, handling rollover to years.
 * @param timer Pointer to the `time_clock` to be incremented.
 */
void clock_incrementar_mes(time_clock_t timer);

/**
 * @brief Increments the year part of a `time_clock` structure.
 * @param timer Pointer to the `time_clock` to be incremented.
 */
void clock_incrementar_year(time_clock_t timer);

/**
 * @brief Increments the min minutes  a `time_clock` structure,
 *  handling rollover to hours.
 * @param timer Pointer to the `time_clock` to be incremented.
 * @param min Integer, minutes of incrments.
 */
void clock_incrementar_min(time_clock_t timer, int min);
/**
 * @brief Increments a specific field (hour, minute, second, day, month, year) of a `time_clock` structure.
 * Handles field-specific rollovers.
 * @param timer Pointer to the `time_clock` to modify.
 * @param campo The index of the field to increment (0=hr, 1=min, 2=sec, 3=day, 4=month, 5=year).
 */
void clock_incrementar_campo(time_clock_t timer, int campo);
/**
 * @brief Decrements a specific field (hour, minute, second, day, month, year) of a `time_clock` structure.
 * Handles field-specific rollovers (e.g., 00:00 -> 23:59 for hours).
 * @param timer Pointer to the `time_clock` to modify.
 * @param campo The index of the field to decrement (0=hr, 1=min, 2=sec, 3=day, 4=month, 5=year).
 */
void clock_decrementar_campo(time_clock_t timer, int campo);


#endif