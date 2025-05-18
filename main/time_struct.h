#ifndef _TIME_SRTUCT_H_
#define _TIME_SRTUCT_H_

typedef enum
{
    UNIDAD = 0,
    DECENA = 1,
    CENTENA = 2
} digito_t;

// tiempo en segundos
typedef struct time_struct
{
    int decima;
    int unidad;
    int decena;
    int centena;
} time_struct;

typedef  struct time_clock
    {
        int year;
        int month;
        int day;
        int hr;
        int min;
        int sec;
    

} time_clock;

typedef struct time_struct *time_struct_t;
typedef struct time_clock *time_clock_t;

void time_cero(time_struct_t timer);

void time_tick(time_struct_t timer);

void time_incrementar_segundo(time_struct_t timer, digito_t digito_inicial);

void clock_init(time_clock_t timer);

void clock_set(time_clock_t timer);

void clock_tick(time_clock_t timer);

#endif