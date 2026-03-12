#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    EVE_NONE = 0,

    EVT_BUTTON,
    EVT_SENSOR_READING,

    EVT_WIFI_UP,
    EVT_WIFI_DOWN,

    EVT_CLOUD_COMMAND,

    EVT_SET_FAN_SPEED,
    EVT_SAVE_CONFIG
} app_event_type_t;

typedef struct {
    app_event_type_t type;

    union {
        struct { // Input Readings
            uint8_t button_id;
            bool pressed;
        } button;

        struct { // Sensor Readings
            float temperature;
            float humidity;
            float pm2_5;
            float pm10;
            bool voc;
            bool is_gas_abnormal;
        } sensor_reading;

        struct { // Storage/AWS command
            char command[64];
        } cloud_command;

        struct {
            uint8_t speed;
        } set_fan_speed;

    } data;
    
} app_event_t;
