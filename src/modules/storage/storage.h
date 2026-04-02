#pragma once
#include "esp_err.h"

esp_err_t storage_init(void);
esp_err_t storage_write_wifi(const char *ssid, const char *pass);
esp_err_t storage_read_wifi(char *ssid, char *pass);
esp_err_t storage_clear_all(void);
