#include "system_monitor.h"
#include "esp_task_wdt.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void system_monitor_task(void *arg) {
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = 5000,
        .idle_core_mask= (1 << 0), // .idle_bitmask = (1 << 0),
        .trigger_panic = true
    };
    esp_task_wdt_init(&twdt_config);

    while (1) {
        size_t free_ram = esp_get_free_heap_size();
        if (free_ram < 10000) {
            ESP_LOGE("SYS", "Low Memory : %u! Restarting...", free_ram);
            esp_restart();
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void system_monitor_init(void) {
    xTaskCreate(system_monitor_task, "sys_mon", 2048, NULL, 1, NULL);
}