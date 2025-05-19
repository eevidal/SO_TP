#include "stdbool.h"
#include "time_struct.h"
#include "freertos/FreeRTOS.h"

int cantidad_dias(int mes, int year)
{
    int days = 0;
    switch (mes)
    {
    case 2: // Febrero
        days = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0) ? 29 : 28;
        break;
    case 4:
    case 6:
    case 9:
    case 11:
        days = 30;
        break;
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        days = 31;
        break;
    }
    return days;
}

void time_cero(time_struct_t timer)
{
    timer->decima = 0;
    timer->unidad = 0;
    timer->decena = 0;
    timer->centena = 0;
}

void time_incrementar_segundo(time_struct_t timer, digito_t digito_inicial)
{
    digito_t digito_actual = digito_inicial;
    bool carry = true;
    while (carry)
    {
        carry = false;
        switch (digito_actual)
        {
        case UNIDAD:
            timer->unidad++;
            if (timer->unidad > 9)
            {
                timer->unidad = 0;
                carry = true;
                digito_actual = DECENA;
            }
            break;
        case DECENA:
            timer->decena++;
            if (timer->decena > 9)
            {
                timer->decena = 0;
                carry = true;
                digito_actual = CENTENA;
            }
            break;
        case CENTENA:
            timer->centena++;
            if (timer->centena > 9)
            {
                timer->unidad = 0;
                timer->decena = 0;
                timer->centena = 0;
                timer->decima = 0;
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

void time_tick(time_struct_t timer)
{
    timer->decima++;
    if (timer->decima == 9)
    {
        timer->decima = 0;
        time_incrementar_segundo(timer, UNIDAD);
    }
};

void clock_init(time_clock_t timer)
{
    timer->year = 2025;
    timer->month = 1;
    timer->day = 1;
    timer->hr = 0;
    timer->min = 0;
    timer->sec = 0;
}

void clock_tick(time_clock_t timer)
{
    timer->sec++;
    if (timer->sec == 60)
    {
        timer->sec = 0;
        clock_incrementar_minuto(timer);
    }
};

void clock_incrementar_minuto(time_clock_t timer)
{
    timer->min++;
    if (timer->min == 60)
    {
        timer->min = 0;
        clock_incrementar_hora(timer);
    }
}

void clock_incrementar_hora(time_clock_t timer)
{
    timer->hr++;
    if (timer->hr == 24)
    {
        timer->hr = 0;
        clock_incrementar_dia(timer);
    }
}

void clock_incrementar_dia(time_clock_t timer)
{
    uint8_t days = cantidad_dias(timer->month, timer->year);
    timer->day++;
    if (timer->day > days)
    {
        timer->day = 1;
        clock_incrementar_mes(timer);
    }
}

void clock_incrementar_mes(time_clock_t timer)
{
    timer->month++;
    if (timer->month == 13)
    {
        timer->month = 0;
        clock_incrementar_year(timer);
    }
}

void clock_incrementar_year(time_clock_t timer)
{
    timer->year++;
}
// 0 -> hr, 1-> min, 2-> seg , 3 -> dia, 4->mes, 5 -> año
void clock_incrementar_campo(time_clock_t timer, int campo)
{
    switch (campo)
    {
    case 0:
        timer->hr++;
        if (timer->hr == 24)
            timer->hr = 0;
        break;
    case 1:
        timer->min++;
        if (timer->min == 60)
            timer->min = 0;
        break;
    case 2:
        timer->sec++;
        if (timer->sec == 60)
            timer->sec = 0;
        break;
    case 3:
        uint8_t days = cantidad_dias(timer->month, timer->year);
        timer->day++;
        if (timer->day > days)
            timer->day = 1;

        break;
    case 4:
        timer->month++;
        if (timer->month > 12)
            ;
        timer->month = 1;
        break;
    case 5:
        timer->year++;
        break;

    default:
        break;
    }
}

void clock_decrementar_campo(time_clock_t timer, int campo)
{
    switch (campo)
    {
    case 0:
        if (timer->hr > 0)
        {
            timer->hr--;
        }
        else
        {
            timer->hr = 23;
        }
        break;
    case 1:
        if (timer->min > 0)
        {
            timer->min--;
        }
        else
        {
            timer->min = 59;
        }
        break;
    case 2:
        if (timer->sec > 0)
        {
            timer->sec--;
        }
        else
        {
            timer->sec = 59;
        }
        break;
    case 3:
        if (timer->day > 1)
        {
            timer->day--;
        }
        else
        {
            timer->day = cantidad_dias(timer->month, timer->year);
        }
        break;
    case 4:
        if (timer->month > 1)
        {
            timer->month--;
        }
        else
        {
            timer->month = 12;
        }
        break;
    case 5:
        timer->year--;
        break;
    default:
        break;
    }
}
