#ifndef _MODE_OP_H
#define _MODE_OP_H

#define MODO_CLOCK 1 << 10
#define MODO_CLOCK_CONF 1 << 11
#define MODO_ALARM 1 << 12
#define MODO_CRONO 1 << 13
#define MODOS MODO_ALARM | MODO_CLOCK | MODO_CRONO | MODO_CLOCK_CONF

typedef enum modos
{
    CLOCK = 0,
    CLOCK_CONF = 1,
    ALARM = 2,
    CRONO = 3

} modos_t;

#endif