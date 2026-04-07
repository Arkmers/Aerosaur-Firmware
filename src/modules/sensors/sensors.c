#include "sensors.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "app/event_bus.h"
#include "dht.h"


#define DHT_PIN 4
#define DHT_TYPE DHT_TYPE_AM2301
#define MQ135_ADC_CH ADC1_CHANNEL_6
#define PMS_RX_PIN 16
#define PMS_TX_PIN 17
#define UART_NUM UART_NUM_2

static const char *TAG = "SENSORS";
extern event_bus_t central_bus;

static bool parse_pms_frame(uint8_t *data, int *pm25, int *pm10) {
    if (data[0] != 0x42 || data[1] != 0x4D) return false;
    *pm25 = (data[6] << 8) | data[7];
    *pm10 = (data[8] << 8) | data[9];
    return true;
}

void sensors_task(void *arg) {
    // ----- Initializing Sensors ----- //
    // --- MQ135 --- //
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(MQ135_ADC_CH, ADC_ATTEN_DB_12);

    // --- PMS5003 --- //
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, PMS_TX_PIN, PMS_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, 128 * 2, 0, 0, NULL, 0);

    uint8_t pms_buf[32];
    
    while(1) {
        app_event_t evt = { .type = EV_SENSOR_DATA };

        // --- DHT22 --- //
        esp_err_t res = dht_read_float_data(DHT_TYPE, DHT_PIN, &evt.data.sensor.hum, &evt.data.sensor.temp);

        if (res != ESP_OK) {
            ESP_LOGW(TAG, "DHT Read Failed: %d", res);
            evt.data.sensor.hum = -99.0;
            evt.data.sensor.temp = -99.0; // possible different error value
        }

        // --- MQ135 --- //
        evt.data.sensor.voc = adc1_get_raw(MQ135_ADC_CH);
        evt.data.sensor.gas_alert = (evt.data.sensor.voc > 1800);

        // --- PMS5003 --- //

        int len = uart_read_bytes(UART_NUM, pms_buf, sizeof(pms_buf), pdMS_TO_TICKS(100));
        if (len >= 32) {
            parse_pms_frame(pms_buf, &evt.data.sensor.pm25, &evt.data.sensor.pm10);
        } else {
            evt.data.sensor.pm25 = -1;
            evt.data.sensor.pm10 = -1;
        }

        ESP_LOGI(TAG, "T:%.1f H:%.1f PM2.5:%d PM10:%d VOC:%d",
            evt.data.sensor.temp,
            evt.data.sensor.hum,
            evt.data.sensor.pm25,
            evt.data.sensor.pm10,
            evt.data.sensor.voc);

        
        event_bus_publish(&central_bus, &evt, pdMS_TO_TICKS(100));

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}