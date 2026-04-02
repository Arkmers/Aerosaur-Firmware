# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(SRCS ${app_sources})

idf_component_register(
    SOURCES
        "main.c"
        "app/event_bus.c"
        "modules/fan_controller/fan_controller.c"
        "modules/input/input.c"
        "modules/sensor/sensors.c"
        "modules/mqtt/mqtt.c"
    INCLUDE_DIRS
        "." "app" "modules"
        "../lib/dht"
        "../lib/esp_idf_lib_helpers"
    REQUIRES
        nvs_flash mqtt driver esp_adc esp_event
        
)