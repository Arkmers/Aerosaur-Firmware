#include "input.h"
#include "driver/gpio.h"
#include "app/event_bus.h"

extern event_bus_t central_bus;

static void IRAM_ATTR btn_isr_handler(void *arg) {
    app_event_t evt = { .type = EV_BTN_PRESSED };
    xQueueSendFromISR(central_bus.q, &evt, NULL);
}

void input_init(int pin) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << pin),
        .pull_up_en = 1
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(pin, btn_isr_handler, NULL);
}