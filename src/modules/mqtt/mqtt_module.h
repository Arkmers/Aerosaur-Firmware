#ifndef MQTT_MODULE_H
#define MQTT_MODULE_H

#include <stdbool.h>

void mqtt_module_init(const char *device_id);
void mqtt_moudle_publish_telemetry(const char *payload);
void mqtt_module_publish_ack(const char *payload);

bool mqtt_module_is_connected(void);

#endif