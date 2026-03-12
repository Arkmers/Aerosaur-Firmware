#include "mqtt_module.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <string.h>
#include "secrets.h"

static const char *TAG = "MQTT_MODULE";

static esp_mqtt_client_handle_t client = NULL;

static char telemetry_topic[128];
static char cmd_topic[128];
static char ack_topic[128];

static char device_id_local[32];

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

void mqtt_module_init(const char *device_id) {
    strncpy(device_id_local, device_id, sizeof(device_id_local));

    snprintf(telemetry_topic, sizeof(telemetry_topic), "devices/%s/readings", device_id);

    snprintf(cmd_topic, sizeof(cmd_topic), "devices/%s/cmd", device_id);

    snprintf(ack_topic, sizeof(ack_topic), "devices/%s/ack", device_id);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = AWS_IOT_ENDPOINT,
        .broker.address.port = 8883,

        .credentials.client_id = device_id,

        .broker.verification.certificate = AWS_CERT_CA,

        .credentials.authentication.certificate = AWS_CERT_CRT,
        .credentials.authentication.key = AWS_CERT_PRIVATE
    };

    client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(
        client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL
    );

    esp_mqtt_client_start(client);

    ESP_LOGI(TAG, "MQTT module initialized");
}

bool mqtt_module_is_connected(void) {

    if (!client) return false;

    return esp_mqtt_client_is_connected(client);

}

void mqtt_module_publish_telemetry(const char *payload) {

    if (!mqtt_module_is_connected()) return;

    esp_mqtt_client_publish(
        client,
        telemetry_topic,
        payload,
        0,
        1,
        0
    );
}

void mqtt_module_publish_ack(const char *payload) {

    if (!mqtt_module_is_connected()) return;

    esp_mqtt_client_publish(
        client,
        ack_topic,
        payload,
        0,
        1,
        0
    );
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {

    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED: {
            ESP_LOGI(TAG, "Connected to AWS IoT");

            esp_mqtt_client_subscribe(
                client,
                cmd_topic,
                1
            );

            ESP_LOGI(TAG, "Subscribed to %s", cmd_topic);

            break;
        }

        case MQTT_EVENT_DISCONNECTED: {
            ESP_LOGW(TAG, "MQTT Disconnected");

            break;
        }

        case MQTT_EVENT_DATA: {
            ESP_LOGI(TAG, "MQTT message received");

            printf("TOPIC: %.*s\n",
                event->topic_len,
                event->topic);

            printf("Data: %.*s\n",
                event->data_len,
                event->data
            );

            break;

        }

        case MQTT_EVENT_ERROR: {
            ESP_LOGE(TAG, "MQTT error");
            
            break;
        }

        default:
        break;
    }
}
