#include "storage_module.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>

static const char *TAG = "STORAGE_MODULE";
static const char *NVS_NAMESPACE = "wifi";

bool storage_module_init(void) {

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Nvs init failed (%d)", ret);
        return false;
    }
    ESP_LOGI(TAG, "NVS Initialized");
    return true;
}

bool storage_module_save_wifi(const char *ssid, const char *pass) {

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return false;

    nvs_set_str(handle, "ssid", ssid);
    nvs_set_str(handle, "pass", pass);

    err = nvs_commit(handle);
    nvs_close(handle);

    if (err = ESP_OK) {
        ESP_LOGI(TAG, "WiFi credentials saved");
        return true;
    }
    ESP_LOGE(TAG, "Failed to save WiFi credentials (%d)", err);
    return false;

}

bool storage_module_load_wifi(char *ssid, size_t ssid_len, char *pass, size_t pass_len) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        nvs_close(handle);
        return false;
    }

    err = nvs_get_str(handle, "pass", pass, &pass_len);
    nvs_close(handle);
    return (err == ESP_OK);
}

bool storage_module_clear_wifi(void) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return false;

    nvs_erase_key(handle, "ssid");
    nvs_erase_key(handle, "pass");

    err = nvs_commit(handle);
    nvs_close(handle);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "WiFi credentials cleared");
        return true;
    }
    ESP_LOGE(TAG, "Failed to clear WiFi credentials (%d)", err);
    return false;
}


