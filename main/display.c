#include "display.h"
#include "time_struct.h"
#include "mode_op.h"

#define RESET_CLOCK_ANT_0()      \
    do                           \
    {                            \
        _clock_ant[0].hr = 0;    \
        _clock_ant[0].min = 0;   \
        _clock_ant[0].sec = 0;   \
        _clock_ant[0].day = 0;   \
        _clock_ant[0].month = 0; \
        _clock_ant[0].year = 0;  \
    } while (0)

#define DIBUJAR_PARCIAL(panel_base, centena, decena, unidad, decima) \
    DibujarDigito(panel_base, 2, unidad);                            \
    DibujarDigito(panel_base, 1, decena);                            \
    DibujarDigito(panel_base, 0, centena);                           \
    DibujarDigito(panel_base##_d, 0, decima);

#define CRONO_RESET_PANTALLA()                                  \
    do                                                          \
    {                                                           \
        ILI9341Fill(DIGITO_APAGADO);                            \
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

#define CLOCK_RESET_PANTALLA()                           \
    do                                                         \
    {                                                          \
        ILI9341Fill(DIGITO_APAGADO);                           \
        DIBUJAR_HORA(rhoras, 0, 0);                            \
        DIBUJAR_HORA(rminutos, 0, 0);                          \
        DIBUJAR_HORA(rsegundos, 0, 0);                         \
        DIBUJAR_HORA(rdia, 0, 0);                              \
        DIBUJAR_MES(rmes, 0, 0);                               \
        DIBUJAR_YEAR(ryear, 0, 0);                             \
        ILI9341DrawFilledCircle(75, 55, 3, DIGITO_ENCENDIDO);  \
        ILI9341DrawFilledCircle(75, 35, 3, DIGITO_ENCENDIDO);  \
        ILI9341DrawFilledCircle(150, 55, 3, DIGITO_ENCENDIDO); \
        ILI9341DrawFilledCircle(150, 35, 3, DIGITO_ENCENDIDO); \
    } while (0);





void dibujar_pantalla(void *args)
{
    int guardados = 0;
    display_task_t display_arg = (display_task_t)args;
    EventGroupHandle_t _event_group = display_arg->event_group;
    QueueHandle_t queue_crono = display_arg->qcrono;
    QueueHandle_t queue_clock = display_arg->qclock;
    QueueHandle_t alarm_clock = display_arg->qalarm;
    int sel = display_arg->selected;
    bool set_alarm = display_arg->selected;
    uint8_t reset_bits = display_arg->reset_bits;
    uint8_t parcial_bits = display_arg->parcial_bits;
    uint8_t event_bits = reset_bits | parcial_bits;

    /*Estructura y variables para el cronometro*/
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

    /*Estructura para guardar un reloj y alarma*/
    /* clock[0] = reloj, clock[1] = alarma*/
    time_clock _clock[2] = {
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0}};
    time_clock _clock_ant[2] = {
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0}};

    ILI9341Init();
    ILI9341Rotate(ILI9341_Portrait_2);

    /*Paneles para el cronómetro*/
    panel_t segundos = CrearPanel(5, 15, 3, DIGITO_ALTO, DIGITO_ANCHO, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t decimas = CrearPanel(188, 15, 1, DIGITO_ALTO, DIGITO_ANCHO, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t parcial1 = CrearPanel(15, 120, 3, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO_R, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t parcial2 = CrearPanel(47, 180, 3, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO_B, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t parcial3 = CrearPanel(80, 240, 3, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO_G, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t parcial1_d = CrearPanel(123, 120, 3, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO_R, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t parcial2_d = CrearPanel(155, 180, 3, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO_B, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t parcial3_d = CrearPanel(188, 240, 3, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO_G, DIGITO_APAGADO, DIGITO_FONDO);

    /*Panel para el reloj y alarma*/
    panel_t rhoras = CrearPanel(5, 15, 2, DIGITO_ALTO_A, DIGITO_ANCHO_A, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t rminutos = CrearPanel(80, 15, 2, DIGITO_ALTO_A, DIGITO_ANCHO_A, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t rsegundos = CrearPanel(155, 15, 2, DIGITO_ALTO_A, DIGITO_ANCHO_A, DIGITO_ENCENDIDO, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t rdia = CrearPanel(15, 120, 2, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO_DG, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t rmes = CrearPanel(47, 180, 2, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO_DG, DIGITO_APAGADO, DIGITO_FONDO);
    panel_t ryear = CrearPanel(80, 240, 4, DIGITO_ALTO_P, DIGITO_ANCHO_P, DIGITO_ENCENDIDO_DG, DIGITO_APAGADO, DIGITO_FONDO);

    CLOCK_RESET_PANTALLA(); // crono
    while (1)
    {
        EventBits_t wBits = xEventGroupWaitBits(_event_group, CAMBIO_MODO | MODOS | event_bits, pdFALSE, pdFALSE, (TickType_t)50);
        switch (wBits & (MODOS))
        {
        case MODO_CLOCK:
            if ((wBits & CAMBIO_MODO)!=0)
            {
                RESET_CLOCK_ANT_0();
                CLOCK_RESET_PANTALLA();
                xEventGroupClearBits(_event_group, CAMBIO_MODO);
            }
            if (xQueueReceive(queue_clock, &(_clock[0]), (TickType_t)50) == pdPASS)
            {
                DIBUJAR_TODO_RELOJ(_clock[0], _clock_ant[0], rhoras, rminutos, rsegundos, rdia, rmes, ryear);
                _clock_ant[0] = _clock[0];
            }
            break;
        case MODO_CLOCK_CONF:
            if ((wBits & CAMBIO_MODO)!=0)
            {
                RESET_CLOCK_ANT_0();
                CLOCK_RESET_PANTALLA();
                xEventGroupClearBits(_event_group, CAMBIO_MODO);
            }

            if (xQueueReceive(queue_clock, &(_clock[0]), (TickType_t)50) == pdPASS)
            {
                DIBUJAR_TODO_RELOJ(_clock[0], _clock_ant[0], rhoras, rminutos, rsegundos, rdia, rmes, ryear);
                _clock_ant[0] = _clock[0];

            }
            break;
        case MODO_ALARM:
            if ((wBits & CAMBIO_MODO)!=0)
            {
                RESET_CLOCK_ANT_0();
                CLOCK_RESET_PANTALLA();
                xEventGroupClearBits(_event_group, CAMBIO_MODO);
            }
            if (xQueueReceive(queue_clock, &(_clock[1]), (TickType_t)50) == pdPASS)
            {
                 DIBUJAR_TODO_RELOJ(_clock[1], _clock_ant[1], rhoras, rminutos, rsegundos, rdia, rmes, ryear);
                _clock_ant[1] = _clock[1];
            }
            break;
        case MODO_ALARM_CONF:
            if ((wBits & CAMBIO_MODO)!=0)
            {
                RESET_CLOCK_ANT_0();
                CLOCK_RESET_PANTALLA();
                xEventGroupClearBits(_event_group, CAMBIO_MODO);
            }

            if (xQueueReceive(alarm_clock, &(_clock[1]), (TickType_t)50) == pdPASS)
            {
                DIBUJAR_TODO_RELOJ(_clock[1], _clock_ant[1], rhoras, rminutos, rsegundos, rdia, rmes, ryear);
                _clock_ant[1] = _clock[1];
            }
            break;
        case MODO_CRONO:
            if ((wBits & CAMBIO_MODO)!=0)
            {
                CRONO_RESET_PANTALLA();
                xEventGroupClearBits(_event_group, CAMBIO_MODO);
            }

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
            if (xQueueReceive(queue_crono, &(tiempo), (TickType_t)50) == pdPASS)
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
            break;
        default:
            break;
        }
    }
}
