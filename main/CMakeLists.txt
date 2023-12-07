idf_component_register(
  SRCS
    src/app_main.cpp
    src/wlan_manager.cpp
    src/utils.cpp
    src/app_nvs.cpp
    src/event_handler.cpp
  INCLUDE_DIRS
    inc
  REQUIRES
    arduino
    etl
    hx711
    cpp-channel
    # https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/mqtt.html#header-file
    mqtt
    # https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html#header-file
    nvs_flash
    nanopb
    tinycbor)

# https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/cplusplus.html
# C++23
target_compile_options(${COMPONENT_LIB} PUBLIC -std=gnu++23)

target_compile_definitions(${COMPONENT_LIB} PRIVATE -DWLAN_AP_SSID=WH-iot
                                                    -DWLAN_AP_PASSWORD=wh213215)
