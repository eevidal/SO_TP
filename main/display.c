#include "display.h"
#include "time_struct.h"

void dibujar_pantalla(void *args)
{
    int guardados = 0;
    display_task_t display_arg = (display_task_t)args;
    EventGroupHandle_t _event_group = display_arg->event_group;
    QueueHandle_t queue_t = display_arg->t;
    uint8_t reset_bits = display_arg->reset_bits;
    uint8_t parcial_bits = display_arg->parcial_bits;
    uint8_t event_bits = reset_bits | parcial_bits;

    ILI9341Init();
    ILI9341Rotate(ILI9341_Portrait_2);

    panel_t segundos = CrearPanel(5, 15, 3, DIGITO_ALTO, DIGITO_ANCHO, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t decimas = CrearPanel(188, 15, 1, DIGITO_ALTO, DIGITO_ANCHO, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t parcial1 = CrearPanel(15, 120, 3, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t parcial2 = CrearPanel(47, 180, 3, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t parcial3 = CrearPanel(80, 240, 3, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t parcial1_d = CrearPanel(123, 120, 3, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t parcial2_d = CrearPanel(155, 180, 3, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t parcial3_d = CrearPanel(188, 240, 3, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);

    time_struct parcial[3] = {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}};
    time_struct tiempo;
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
    ILI9341DrawFilledCircle(178, 95, 5, DIGITO_ENCENDIDO);
    ILI9341DrawFilledCircle(113, 160, 5, DIGITO_ENCENDIDO);
    ILI9341DrawFilledCircle(145, 220, 5, DIGITO_ENCENDIDO);
    ILI9341DrawFilledCircle(178, 280, 5, DIGITO_ENCENDIDO);
    DIBUJAR_PARCIAL(parcial1, 0, 0, 0, 0);
    DIBUJAR_PARCIAL(parcial2, 0, 0, 0, 0);
    DIBUJAR_PARCIAL(parcial3, 0, 0, 0, 0);
    while (1)
    {
        EventBits_t wBits = xEventGroupWaitBits(_event_group, event_bits, pdFALSE, pdFALSE, (TickType_t)0);

        if ((wBits & reset_bits) != 0)
        {
            xEventGroupClearBits(_event_group, reset_bits);
            DIBUJAR_PARCIAL(parcial1, 0, 0, 0, 0);
            DIBUJAR_PARCIAL(parcial2, 0, 0, 0, 0);
            DIBUJAR_PARCIAL(parcial3, 0, 0, 0, 0);
            guardados = 0;
            parcial[0].unidad = 0;
            parcial[0].decena = 0;
            parcial[0].centena = 0;
            parcial[0].decima = 0;
            parcial[1] = parcial[0];
            parcial[2] = parcial[0];
        }
        if (xQueueReceive(queue_t, &(tiempo), (TickType_t)50) == pdPASS)
        {
            unidad_act = tiempo.unidad;
            decena_act = tiempo.decena;
            centena_act = tiempo.centena;
            decima_act = tiempo.decima;

            DIBUJAR_SI_CAMBIA(unidad_act, unidad_ant, segundos, 2);
            DIBUJAR_SI_CAMBIA(decena_act, decena_ant, segundos, 1);
            DIBUJAR_SI_CAMBIA(centena_act, centena_ant, segundos, 0);
            DIBUJAR_SI_CAMBIA(decima_act, decima_ant, decimas, 0);

            decima_ant = decima_act;
            unidad_ant = unidad_act;
            decena_ant = decena_act;
            centena_ant = centena_act;

            if ((wBits & parcial_bits) != 0)
            {
                guardados < 3 ? guardados++ : guardados;
                int i;
                for (i = guardados; i > 1; i--)
                {
                    parcial[i - 1] = parcial[i - 2];
                }
                parcial[0].unidad = unidad_ant;
                parcial[0].decena = decena_ant;
                parcial[0].centena = centena_ant;
                parcial[0].decima = decima_ant;
                xEventGroupClearBits(_event_group, parcial_bits);

                DIBUJAR_PARCIAL(parcial1, parcial[0].centena,
                                parcial[0].decena,
                                parcial[0].unidad, parcial[0].decima);
                DIBUJAR_PARCIAL(parcial2, parcial[1].centena,
                                parcial[1].decena,
                                parcial[1].unidad, parcial[1].decima);
                DIBUJAR_PARCIAL(parcial3, parcial[2].centena,
                                parcial[2].decena,
                                parcial[2].unidad, parcial[2].decima);
            }
        }
    }
}
