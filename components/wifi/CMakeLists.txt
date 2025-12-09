idf_component_register(SRCS "wifi.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_netif esp_event lwip)

target_compile_definitions(${COMPONENT_LIB} PUBLIC
    -DWIFI_SSID=\"309Study\"
    -DWIFI_PASSWORD=\"ai12321\"
)
