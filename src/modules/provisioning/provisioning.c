#include "sdkconfig.h"
#include "provisioning.h"
#include <string.h>
#include "esp_log.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "cJSON.h"
#include "app/event_bus.h"

static const char *TAG = "PROVISIONING";
extern event_bus_t central_bus;

static const ble_uuid128_t SVC_UUID =
    BLE_UUID128_INIT(0x9e, 0xca, 0x2c, 0xd4, 0xe0, 0x5e, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e);

static const ble_uuid128_t CH_WIFI_UUID =
    BLE_UUID128_INIT(0x9e, 0xca, 0x2c, 0xd4, 0xe0, 0x5e, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e);

static int wifi_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &SVC_UUID.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = &CH_WIFI_UUID.u,
                .access_cb = wifi_access_cb,
                .flags = BLE_GATT_CHR_F_WRITE,
            },
            {0}
        },
    },
    {0}
};


static int wifi_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        char json_buf[128] = {0};
        uint16_t len = OS_MBUF_PKTLEN(ctxt->om);
        if (len >= sizeof(json_buf)) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

        ble_hs_mbuf_to_flat(ctxt->om, json_buf, len, NULL);

        ESP_LOGI(TAG, "Received Data: %s", json_buf);

        cJSON *root = cJSON_Parse(json_buf);
        if (root) {
            cJSON *ssid = cJSON_GetObjectItem(root, "ssid");
            cJSON *pass = cJSON_GetObjectItem(root, "pass");

            if (ssid && cJSON_IsString(ssid)) {
                app_event_t evt = { .type = EV_WIFI_PROV_RECV };
                strncpy(evt.data.wifi_prov.ssid,  ssid->valuestring, 32);
                strncpy(evt.data.wifi_prov.pass, pass ? pass->valuestring : "", 64);

                event_bus_publish(&central_bus, &evt, 0);
            }
            cJSON_Delete(root);
        }
    }
    return 0;
}

static void ble_on_sync(void) {
    uint8_t addr_type;
    ble_hs_id_infer_auto(0, &addr_type);

    struct ble_gap_adv_params adv_params = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_GEN,
    };

    struct ble_hs_adv_fields fields = {0};
    fields.name = (uint8_t *)"AIR203-PROV";
    fields.name_len = strlen((char *)fields.name);
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    ble_gap_adv_set_fields(&fields);
    ble_gap_adv_start(addr_type, NULL, BLE_HS_FOREVER, &adv_params, NULL, NULL);
    ESP_LOGI(TAG, "Advertising Started...");
}

void provisioning_host_task(void *param) {
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void provisioning_start(void) {
    ESP_LOGI(TAG, "Initializing BLE Provisioning...");

    nimble_port_init();
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);

    ble_hs_cfg.sync_cb = ble_on_sync;

    nimble_port_freertos_init(provisioning_host_task);
}

void provisioning_stop(void) {
    nimble_port_stop();
}