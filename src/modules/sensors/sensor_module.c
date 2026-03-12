#include "sensor_module.h"

#include "driver/adc.h"
#include "driver/uart.h"
#include "dht.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string.h>
#include <stdio.h>


// ----- Configuration ----- //

#define DHT_GPIO GPIO_NUM_4
#define DHT_TYPE DHT_TYPE_AM2301

#define MQ135_ADC_CH ADC1_CHANNEL_6

#define PMS_UART_NUM UART_NUM_2
#define PMS_RX_GPIO 16
#define PMS_TX_GPIO 17

#define GAS_THRESHOLD 1800
#define PMS_UART_BUF_LEN 32

static const char *TAG = "SENSOR";

// ----- Module State ----- //

static event_bus_t *g_bus = NULL;

static uart_port_t pms_uart = PMS_UART_NUM;
static uint8_t uart_buf[PMS_UART_BUF_LEN];

static float last_temp = 0;
static float last_humidity = 0;
static uint64_t last_dht_read = 0;

void sensors_hw_init(void) {

    // MQ135 ADC Init
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(MQ135_ADC_CH, ADC_ATTEN_DB_11);

    // PMS UART Init
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    
    uart_param_config(pms_uart, &uart_config);
    uart_set_pin(pms_uart, PMS_TX_GPIO, PMS_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(pms_uart, PMS_UART_BUF_LEN * 2, 0, 0, NULL, 0);
}

// ----- Sensor Reads ----- //

static void read_dht(void) { // DHT

    uint64_t now = esp_timer_get_time();

    if (now - last_dht_read < 2000000) return;

    if (dht_read_float_data(DHT_TYPE, DHT_GPIO, &last_humidity, &last_temp) != ESP_OK) {
        ESP_LOGW(TAG, "Failed reading DHT22");
    }
    last_dht_read = now;

}

static int read_gas(void) { // MQ135

    return adc1_get_raw(MQ135_ADC_CH);

}

static bool read_pm(int *pm2_5, int *pm10) {

    int len = uart_read_bytes(pms_uart, uart_buf, PMS_UART_BUF_LEN, 100 / portTICK_PERIOD_MS);

    if (len != PMS_UART_BUF_LEN) return false;

    if (uart_buf[0] != 0x42 || uart_buf[1] != 0x4D) return false;

    *pm2_5 = (uart_buf[12] << 8) | uart_buf[13];
    *pm10 = (uart_buf[14] << 8) | uart_buf[15];

    return true;

}

static void sensor_task(void *arg) {

    sensors_hw_init();

    while(1) {

        read_dht();

        int gas = read_gas();

        int pm2_5 = 0;
        int pm10 = 0;
        bool pm_ok = read_pm(&pm2_5, &pm10);

        app_event_t evt = {
            .type = EVT_SENSOR_READING
        };

        evt.data.sensor_reading.temperature = last_temp;
        evt.data.sensor_reading.humidity = last_humidity;

        evt.data.sensor_reading.pm2_5 = pm_ok ? pm2_5 : -1;
        evt.data.sensor_reading.pm10 = pm_ok ? pm10 : -1;

        evt.data.sensor_reading.voc = gas > GAS_THRESHOLD;
        evt.data.sensor_reading.is_gas_abnormal = gas > GAS_THRESHOLD;

        event_bus_publish(g_bus, &evt, portMAX_DELAY);

        ESP_LOGI(TAG,
        "Temp=%.1fC Hum=%.1f%% PM2.5=%d PM10%d Gas=%d",
        last_temp,
        last_humidity,
        pm2_5,
        pm10,
        gas
    );

    vTaskDelay(pdMS_TO_TICKS(2000));

    }

}

// ----- Module Init ----- //

void sensor_module_init(event_bus_t *bus) {

    g_bus = bus;

    xTaskCreate(
        sensor_task,
        "sensor_task",
        4096,
        NULL,
        10,
        NULL
    );
    
}