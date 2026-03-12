#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app/event_bus.h"
#include "internal.h"

static event_bus_t *g_bus = NULL;

static void internal_task(void *arg) {
    (void) arg;

    app_event_t evt;

    while (1) {
        if (!event_bus_wait(g_bus, &evt, portMAX_DELAY)) continue;

        switch (evt.type) {
            case EVT_SENSOR_READING:
                printf("Sensor read");
                break;
            case EVT_SET_FAN_SPEED:
                printf("Fan speed changed");
                break;
            default:
                break;
        }
    }
}

void internal_init(event_bus_t *bus) {
    g_bus = bus;
    
    xTaskCreate(internal_task, "internal", 4096, NULL, 10, NULL);
}