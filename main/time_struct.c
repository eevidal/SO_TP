#include "stdbool.h"
#include "time_struct.h"

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