#include "event_bus.h"

bool event_bus_init(event_bus_t *bus, size_t queue_length) {
    bus->q = xQueueCreate(queue_length, sizeof(app_event_t));
    return bus->q != NULL;
}

bool event_bus_publish(event_bus_t *bus, const app_event_t *event, TickType_t ticks) {
    if (bus->q == NULL) return false;
    return xQueueSend(bus->q, event, ticks) == pdPASS;
}

bool event_bus_wait(event_bus_t *bus, app_event_t *event, TickType_t ticks) {
    if (bus->q == NULL) return false;
    return xQueueReceive(bus->q, event, ticks) == pdPASS;
}