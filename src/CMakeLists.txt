file(GLOB_RECURSE SOURCES "*.c")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS 
        "." 
        "app" 
        "modules"
        "modules/storage"
        "modules/connectivity"
        "modules/provisioning"
        "modules/mqtt"
        "modules/sensors"
        "modules/fan_controller"
        "modules/input"
        "modules/system_monitor"
        "../lib/dht"
    REQUIRES 
        nvs_flash 
        mqtt 
        driver 
        esp_adc 
        esp_event 
        esp_wifi 
        esp_netif
        esp_timer
        bt
        lwip
        json
        esp_hw_support
        esp_system
        log
)