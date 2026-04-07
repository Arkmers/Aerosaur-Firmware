#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    EV_WIFI_STATUS,
    EV_BTN_PRESSED,
    EV_SENSOR_DATA,
    EV_MQTT_CMD,
    EV_WIFI_PROV_RECV,
    EV_SYS_RESET
} app_event_type_t;

typedef struct {
    app_event_type_t type;
    union {
        struct {
            float temp;
            float hum;
            int pm25;
            int pm10;
            int voc;
            bool gas_alert;
        } sensor;
        struct {
            bool power;
            int speed_idx;
            char cmd_id[36];
        } fan;
        struct {
            char ssid[32];
            char pass[64];
        } wifi_prov;

        bool wifi_status;
    } data;
} app_event_t;
