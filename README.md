# Requerimientos: 
Implementar un cronómetro utilizando más de una tarea de FreeRTOS que muestre en pantalla el valor de la cuenta actual con una resolución de décimas de segundo.

El cronómetro debe iniciar y detener la cuenta al presionar un pulsador conectado a la placa. Si está detenido, al presionar un segundo contador debe volver a cero. 

Mientras la cuenta está activa un led RGB debe parpadear en verde y cuando está detenida debe permanecer en rojo. 

## Estructura de directorio del proyecto

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
└── README.md                  This is the file you are currently reading
```

