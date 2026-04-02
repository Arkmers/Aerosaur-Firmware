#pragma once
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "app_event.h"

typedef struct {
    QueueHandle_t q;
} event_bus_t;

bool event_bus_init(event_bus_t *bus, size_t queue_length);
bool event_bus_publish(event_bus_t *bus, const app_event_t *event, TickType_t ticks_to_wait);
bool event_bus_wait(event_bus_t *bus, app_event_t *event, TickType_t ticks_to_wait);

    