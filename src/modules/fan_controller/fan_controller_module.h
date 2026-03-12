#pragma once

#include "app/event_bus.h"

void fan_controller_init(event_bus_t *bus);

void fan_controller_set_speed_percent(int percent);

int fan_controller_get_speed_percent(void);