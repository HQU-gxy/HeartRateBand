idf_component_register(SRCS src/app_main.cpp INCLUDE_DIRS inc)

# https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/cplusplus.html
# C++23
target_compile_options(${COMPONENT_LIB} PUBLIC -std=gnu++23)

target_compile_definitions(${COMPONENT_LIB} PRIVATE -DWLAN_AP_SSID=WH-iot
                                                    -DWLAN_AP_PASSWORD=wh213215)
