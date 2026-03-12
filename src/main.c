#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "app/event_bus.h"
#include "modules/internal/internal.h"
#include "modules/sensors/sensor_module.h"
#include "modules/fan_controller/fan_controller_module.h"

void app_main(void) {
    static event_bus_t bus;

    if (!event_bus_init(&bus, 32)) {
        printf("Failed to init event bus\n");
        return;
    }

    internal_init(&bus);
    sensors_init(&bus);
    fan_controller_init(&bus);

    printf("App Started\n");
}
