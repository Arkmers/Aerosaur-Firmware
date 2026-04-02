#pragma once
#include "app/event_bus.h"

void mqtt_init(void);
void mqtt_send_telemetry(app_event_t *evt);