#ifndef _TECLAS_H_
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"


/**
 * @brief Structure to hold parameters for the key (button) monitoring task.
 */
typedef struct key_task{
    EventGroupHandle_t event_group; /**< Handle to the FreeRTOS event group to set bits in. */
    uint16_t event_bit; /**< The specific bit to set in the event group when the key is pressed. */
    gpio_num_t gpio_id;  /**< The GPIO pin number to monitor for key presses. */
    

} key_task;


/**
 * @brief Pointer to the key_task structure.
 *//**
 * @brief FreeRTOS task to monitor a GPIO pin for a button press and signal an Event Group.
 *
 * This task continuously monitors the state of a specified GPIO pin, configured as an input
 * with an internal pull-up resistor. When it detects a button press (a transition from high to low),
 * it sets a predefined bit in a FreeRTOS Event Group.
 *
 * The task incorporates debouncing by delaying for 100ms after detecting a level change
 * to ensure stable readings and prevent multiple events from a single press.
 *
 * @param args A pointer to a `key_task_t` structure containing the configuration
 * parameters for the key task, including the GPIO ID to monitor, the event group handle,
 * and the specific event bit to set.
 */
typedef struct key_task * key_task_t;


void tarea_tecla(void * args);


#endif