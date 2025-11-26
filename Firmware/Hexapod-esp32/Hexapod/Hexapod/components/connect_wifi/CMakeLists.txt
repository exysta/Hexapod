idf_component_register(SRCS "connect_wifi.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES
                    esp_wifi
                    esp_netif
                    nvs_flash
                    freertos
                    driver
                    lwip
                    esp_event
                    esp_system
                    log
                    )
