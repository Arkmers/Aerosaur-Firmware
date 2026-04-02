#include "fan_controller.h"
#include "driver/ledc.h"

static const int SPEED_STEPS[] = {0, 40, 41, 60};

void fan_init(int pin) {
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 25000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t chan = {
        .gpio_num = pin,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 1023 // OFF
    };
    ledc_channel_config(&chan);
}

void fan_set_speed_step(int step_idx) {
    if (step_idx < 0 || step_idx > 3) return;
    int duty = (SPEED_STEPS[step_idx] * 1023) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1023 - duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}