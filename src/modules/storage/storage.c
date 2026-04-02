#include "storage.h"
#include "nvs_flash.h"
#include "string.h"

esp_err_t storage_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

esp_err_t storage_write_wifi(const char *ssid, const char *pass) {
    nvs_handle_t h;
    if (nvs_open("storage", NVS_READWRITE, &h) != ESP_OK) return ESP_FAIL;
    nvs_set_str(h, "ssid", ssid);
    nvs_set_str(h, "pass", pass);
    nvs_commit(h);
    nvs_close(h);
    return ESP_OK;
}

esp_err_t storage_read_wifi(char *ssid, char *pass) {
    nvs_handle_t h;
    if (nvs_open("storage", NVS_READONLY, &h) != ESP_OK) return ESP_FAIL;
    size_t s1 = 32, s2 = 64;
    nvs_get_str(h, "ssid", ssid, &s1);
    nvs_get_str(h, "pass", pass, &s2);
    nvs_close(h);
    return (strlen(ssid) > 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t storage_clear_all(void) {
    return nvs_flash_erase();
}

