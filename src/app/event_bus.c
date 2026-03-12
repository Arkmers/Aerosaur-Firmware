#include "event_bus.h"

bool event_bus_init(event_bus_t *bus, uint32_t queue_len) {
    if (!bus) return false;

    bus->q = xQueueCreate((UBaseType_t)queue_len, sizeof(app_event_t));
    return bus->q != NULL;
}

bool event_bus_publish(event_bus_t *bus, const app_event_t *event, TickType_t ticks_to_wait) {
    if (!bus || !bus->q || !event) return false;

    return xQueueSend(bus->q, event, ticks_to_wait) == pdTRUE;
}

bool event_bus_wait(event_bus_t *bus, app_event_t *event, TickType_t ticks_to_wait) {
    if (!bus || !bus->q || !event) return false;

    return xQueueReceive(bus->q, event, ticks_to_wait) == pdTRUE;
}