#include "fan_controller_module.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/ledc.h"
#include "driver/gpio.h"

#include <stdio.h>

// ----- Configuration ----- //

#define FAN_PWM_PIN 27
#define FAN_PWM_FREQ 25000
#define FAN_PWM_RES 10

static event_bus_t *g_bus = NULL;

static int current_speed = 0;

// ----- Module State ----- //

static void fan_hw_init(void) {
    
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = FAN_PWM_RES,
        .freq_hz = FAN_PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num = FAN_PWM_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    ledc_channel_config(&ledc_channel);

}

// ----- Fan Controller ----- //

void fan_controller_set_speed_percent(int percent) {

    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    current_speed = percent;

    int maxDuty = (1 << FAN_PWM_RES) - 1;

    int duty = (maxDuty * (100 - percent)) / 100;

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

}

static void fan_task(void *arg) {

    app_event_t evt;

    while (1) {
        if (!event_bus_wait(g_bus, &evt, portMAX_DELAY)) continue;

        switch (evt.type) {
            case EVT_SET_FAN_SPEED:
                fan_controller_set_speed_percent(
                    evt.data.set_fan_speed.speed
                );

                printf("Fan speed set to %d%%\n",
                        evt.data.set_fan_speed.speed);
                break;
            
                default:
                    break;
        }

    }
    
}

// ----- Module Init ----- //

void fan_controller_init(event_bus_t *bus) {
    
    g_bus = bus;

    fan_hw_init();

    xTaskCreate(
        fan_task,
        "fan_task",
        4096,
        NULL,
        10,
        NULL
    );
    
}