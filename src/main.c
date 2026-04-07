#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

// --- App Core --- //
#include "app/app_event.h"
#include "app/event_bus.h"

// --- Modules --- //
#include "modules/storage/storage.h"
#include "modules/connectivity/connectivity.h"
#include "modules/mqtt/mqtt_module.h"
#include "modules/sensors/sensors.h"
#include "modules/fan_controller/fan_controller.h"
#include "modules/input/input.h"
#include "modules/system_monitor/system_monitor.h"
#include "modules/provisioning/provisioning.h"

static const char *TAG = "AEROSAUR_CORE";

event_bus_t central_bus;

static int fan_step = 0;
static bool is_wifi_connected = false;

void app_main(void) {
    ESP_LOGI(TAG, "Starting Aerosaur AIR203...");

    // --- Initialize Storage NVS --- //
    if (storage_init() != ESP_OK) {
        ESP_LOGE(TAG, "Storaige initialization failed!");
    }

    // --- Initialize Event Bus Thread-safe communication --- //
    if (!event_bus_init(&central_bus, 40)) {
        ESP_LOGE(TAG, "Event Bus creation failed!");
        return;
    }

    // --- Initialize Actuators and Inputs --- //
    fan_init(27);           // Fan PWM on Pin 27
    input_init(14);         // Button on Pin 14
    system_monitor_init();  // Health and Watchdog
    
    // --- Determine Boot Mode (WiFi vs BLE Provisioning) --- //
    char ssid[32] = {0};
    char pass[64] = {0};

    if (storage_read_wifi(ssid, pass) == ESP_OK) {
        ESP_LOGI(TAG, "Stored credentials found. Connecting to: %s", ssid);
        connectivity_init(ssid, pass);
    } else {
        ESP_LOGI(TAG, "No credentials. Starting BLE Provisioning...");
        // Provisioning start function here
    }

    // --- Initialize Independent Producer Tasks --- //
    xTaskCreate(sensors_task, "sensors_task", 4096, NULL, 5, NULL);

    // --- Main Event Dispatcher --- //
    app_event_t event;
    while (1) {
        if (event_bus_wait(&central_bus, &event, portMAX_DELAY)) {

            switch (event.type) {
                
                case EV_WIFI_PROV_RECV:
                    ESP_LOGI(TAG, "BLE Provisioning Received! Saving...");
                    storage_write_wifi(event.data.wifi_prov.ssid, event.data.wifi_prov.pass);
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    esp_restart();
                    break;

                case EV_WIFI_STATUS:
                    if (event.data.wifi_status && !is_wifi_connected) {
                        ESP_LOGI(TAG, "Wifi Connected! Initializing MQTT...");
                        is_wifi_connected = true;
                        mqtt_init();
                    }
                    break;

                case EV_BTN_PRESSED:
                    fan_step = (fan_step + 1) % 4;
                    fan_set_speed_step(fan_step);
                    ESP_LOGI(TAG, "Button Triggered: Fan Step %d", fan_step);
                    break;

                case EV_SENSOR_DATA:
                    if (is_wifi_connected) mqtt_send_telemetry(&event);
                    break;

                case EV_MQTT_CMD:
                    ESP_LOGI(TAG, "MQTT Command Received for Fan: %d", event.data.fan.speed_idx);
                    fan_set_speed_step(fan_step);
                    break;

                case EV_SYS_RESET:
                    ESP_LOGW(TAG, "System Reset Requested. Wiping NVS...");
                    storage_clear_all();
                    vTaskDelay(pdMS_TO_TICKS(500));
                    esp_restart();
                    break;

                default:
                    ESP_LOGW(TAG, "Unhandled Event Type: %d", event.type);
                    break;
            }
        }
    }
}

