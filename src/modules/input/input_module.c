#include "freertos/task.h"
#include "core/device_event_bus.h"
#include "esp_event.h"

#include "input_module.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "INPUT_MODULE";

static int btn_pin = 14;
static int debounce_time = 180;

static int last_read = 1;
static int last_stable = 1;
static int fan_pressed_flag = 0;

static int setup_mode_flag = 0;
static int setup_check_started = 0;

static int64_t last_change_time = 0;
static int64_t setup_start_time = 0;

void input_module_init(int btn_gpio, int debounce_ms) {
    btn_pin = btn_gpio;
    debounce_time = debounce_ms;

    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << btn_pin,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "Button GPIO %d initialized", btn_pin);

    last_change_time = esp_timer_get_time();

    if (gpio_get_level(btn_pin) == 0) {
        setup_check_started = 1;
        setup_start_time = esp_timer_get_time();
    }

    xTaskCreate(
        "input_task",
        2048,
        NULL,
        5,
        NULL
    );
}

void input_module_update(void) {

    int reading = gpio_get_level(btn_pin);
    int64_t now = esp_timer_get_time();

    if (reading != last_read) {
        last_read = reading;
        last_change_time = now;
    }

    if ((now - last_change_time) > (debounce_time * 1000)) {

        if (reading != last_stable) {
            last_stable = reading;

            if (last_stable == 0) {
                fan_pressed_flag = 1;
            }
        }
    }

    if (!setup_mode_flag && reading == 0) {
        if (!setup_check_started) {
            setup_check_started = 1;
            setup_start_time = now;
        }
        else if ((now - setup_start_time) >= 3000000) {
            setup_mode_flag = 1;
            ESP_LOGI(TAG, "Setup mode triggered");
        }
    }
    else if (reading == 1) {
        setup_check_started = 0;
    }
}

bool input_module_is_setup_mode(void) {
    return setup_mode_flag;
}

bool input_module_fan_pressed(void) {
    if (fan_pressed_flag) {
        fan_pressed_flag = 0;
        return true;
    }
    return false;
}

static void input_task(void *arg) {
    while (1) {
        input_module_update();

        if (input_module_fan_pressed()) {
            esp_event_post(
                DEVICE_EVENTS,
                DEVICE_EVENT_FAN_BUTTON,
                NULL,
                0,
                portMAX_DELAY
            );
        }

        if (input_module_is_setup_mode()) {
            esp_event_post(
                DEVICE_EVENTS,
                DEVICE_EVENT_SETUP_MODE,
                NULL,
                0,
                portMAX_DELAY
            );
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
