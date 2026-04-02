#include "mqtt_module.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "secrets.h"
#include "app/event_bus.h"

static const char *TAG = "MQTT_MODULE";
static esp_mqtt_client_handle_t client;
extern event_bus_t central_bus;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Connected to AWS IoT");
            esp_mqtt_client_subscribe(client, "devices/" DEVICE_ID "/cmd", 1);
            break;
        
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Command Received!");
            cJSON *root = cJSON_ParseWithLength(event->data, event->data_len);
            if (root) {
                app_event_t out_evt = { .type = EV_MQTT_CMD };
                
                cJSON *speed = cJSON_GetObjectItem(root, "fanSpeed");
                cJSON *pwr = cJSON_GetObjectItem(root, "power");

                if (speed) out_evt.data.fan.speed_idx = speed->valueint;
                if (pwr) out_evt.data.fan.power = cJSON_IsTrue(pwr);

                event_bus_publish(&central_bus, &out_evt, 0);
                cJSON_Delete(root);
            }
            break;
        
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT Error Occurred");
            break;

        default:
            break;
    }
}

void mqtt_init(void) {
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = AWS_IOT_ENDPOINT,
            .address.port = AWS_IOT_PORT,
            .verification.certificate = AWS_CERT_CA,
        },
        .credentials = {
            .client_id = DEVICE_ID,
            .authentication = {
                .certificate = AWS_CERT_CRT,
                .key = AWS_CERT_PRIVATE,
            },
        }
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void mqtt_send_telemetry(app_event_t *evt) {
    if (!client) return;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "deviceId", DEVICE_ID);
    cJSON_AddNumberToObject(root, "tempC", evt->data.sensor.temp);
    cJSON_AddNumberToObject(root, "humidity", evt->data.sensor.hum);
    cJSON_AddNumberToObject(root, "pm25", evt->data.sensor.pm25);
    cJSON_AddNumberToObject(root, "pm10", evt->data.sensor.pm10);
    cJSON_AddNumberToObject(root, "voc", evt->data.sensor.voc);
    cJSON_AddBoolToObject(root, "gasAlert", evt->data.sensor.gas_alert);
        
    char *post_data = cJSON_PrintUnformatted(root);

    esp_mqtt_client_publish(client, "devices/" DEVICE_ID "/readings", post_data, 0, 1, 0);

    ESP_LOGI(TAG, "Telemetry Published: %s", post_data);

    cJSON_free(post_data);
    cJSON_Delete(root);
    
}