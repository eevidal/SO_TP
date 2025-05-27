#ifndef _LEDS_H_
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

/**
 * @brief Structure to hold parameters for the LED control task.
 */
typedef struct led_task
{
    EventGroupHandle_t event_group; /**< Handle to the FreeRTOS event group used for LED control. */
    uint8_t mask_verde;             /**< Bit mask for the green LED event. */
    uint8_t mask_red;               /**< Bit mask for the red LED event. */
    gpio_num_t gpio_id_red;         /**< GPIO pin number for the red LED. */
    gpio_num_t gpio_id_verde;       /**< GPIO pin number for the green LED. */
    uint16_t tiempo;                /**< Delay time in milliseconds for the green LED blinking. */
} led_task;

/**
 * @brief Pointer to the led_task structure.
 */
typedef struct led_task *led_task_t;

/**
 * @brief FreeRTOS task to control LEDs based on events received from an Event Group.
 *
 * This task initializes and controls two LEDs (red and green) connected to specified GPIO pins.
 * It waits for events from an Event Group to change the state of the LEDs.
 * - If the `mask_red` event bit is set, the red LED turns on and the green LED turns off.
 * - If the `mask_verde` event bit is set, the red LED turns off, and the green LED blinks
 * for a duration specified by `tiempo`. The green LED will turn off after blinking.
 * - If neither `mask_red` nor `mask_verde` are set, both LEDs are turned off.
 *
 * The task uses `xEventGroupWaitBits` with a timeout to periodically check for events.
 * It also uses `vTaskDelayUntil` to maintain precise timing for the green LED blinking.
 *
 * @param args A pointer to a `led_task_t` structure containing the configuration
 * parameters for the LED task, including GPIO IDs, event group handle,
 * event masks, and blinking time.
 */
void tarea_led(void *args);

#endif