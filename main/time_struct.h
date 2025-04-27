#ifndef _TIME_SRTUCT_H_
#define _TIME_SRTUCT_H_

typedef enum
{
    UNIDAD = 0,
    DECENA = 1,
    CENTENA = 2
} digito_t;

typedef struct time_struct
{
    int decima;
    int unidad;
    int decena;
    int centena;
} time_struct;
typedef struct time_struct *time_struct_t;

void time_cero(time_struct_t timer);

void time_tick(time_struct_t timer);

void time_incrementar_segundo(time_struct_t timer, digito_t digito_inicial);



#endif