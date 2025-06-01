# Reloj y Cronómetro
## Requerimientos TP1: 
Implementar un cronómetro utilizando más de una tarea de FreeRTOS que muestre en pantalla el valor de la cuenta actual con una resolución de décimas de segundo.

El cronómetro debe iniciar y detener la cuenta al presionar un pulsador conectado a la placa. Si está detenido, al presionar un segundo contador debe volver a cero. 

Mientras la cuenta está activa un led RGB debe parpadear en verde y cuando está detenida debe permanecer en rojo. 

## Requerimientos TP2:
Modificar el cronometro para registrar por lo menos dos tiempos parciales, los que se mostraran por pantalla simultaneamente con el valor de la cuenta principal.
En la implementación se deberan utilizar eventos y colas.

## Requerimientos TP Final:
TP Final: Se deberá agregar al cronometo del TP2 la funcionalidad de reloj desperdiaor. Se deberá poder ajustar la hora del reloj y la hora de la alarma. La alarma se deberá poder deshabilitar, posponer por 5 minutos o apagar hasta el dia siguiente.

## Estructura de directorio del proyecto para el TP2

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   ├── LICENSE
│   ├── digitos.c
│   ├── digitos.h
│   ├── fonts.c
│   ├── fonts.h
│   ├── ili9341.c
│   ├── ili9341.h
│   └── main.c
└── README.md                
```

## Estructura de directorio del proyecto para el TP Final

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   ├── LICENSE
│   ├── digitos.c
│   ├── digitos.h
│   ├── display.c
│   ├── display.h
│   ├── fonts.c
│   ├── fonts.h
│   ├── ili9341.c
│   ├── ili9341.h
│   ├── leds.c
│   ├── leds.h
│   └── main.c
│   ├── mode_op.h
│   ├── teclas.c
│   ├── teclas.h
│   ├── time_struct.c
│   ├── time_struct.h
└── README.md                
```

Link video demo: 
https://www.youtube.com/watch?v=rwVjhiHdGc0
