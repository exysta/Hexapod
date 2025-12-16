idf_component_register(SRCS "web-server.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES driver connect_wifi esp_http_server nvs_flash spiffs esp_wifi web-server spi_flash led_strip json

                    )
