#ifndef _MODE_OP_H
#define _MODE_OP_H

#define MODO_CLOCK 1 << 10
#define MODO_CLOCK_CONF 1 << 11
#define MODO_ALARM 1 << 12
#define MODO_ALARM_CONF 1 << 13
#define MODO_CRONO 1 << 14
#define MODOS MODO_ALARM | MODO_CLOCK | MODO_CRONO | MODO_CLOCK_CONF | MODO_ALARM_CONF
#define CAMBIO_MODO 1 << 15  // indica que hubo un cambio de modo
typedef enum modos
{
    CLOCK = 1 << 10,
    CLOCK_CONF = 1 << 11,
    ALARM = 1 << 12,
    ALARM_CONF =1 << 13,
    CRONO = 1 << 14
} modos_t;



#endif