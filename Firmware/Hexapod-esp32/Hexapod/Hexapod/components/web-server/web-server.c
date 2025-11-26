#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include "driver/gpio.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_server.h"

#include "connect_wifi.h"
#include "web-server.h"

#define LED_PIN 2

static const char *TAG = "WebSocketServer";
static httpd_handle_t server = NULL;
static int led_state = 0;

#define INDEX_HTML_PATH "/spiffs/index.html"
static char *index_html = NULL;

// ---------------------------------------------------------
// Load webpage from SPIFFS
// ---------------------------------------------------------
static esp_err_t init_web_page_buffer(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    struct stat st;
    if (stat(INDEX_HTML_PATH, &st) != 0) {
        ESP_LOGE(TAG, "index.html not found");
        return ESP_FAIL;
    }

    index_html = malloc(st.st_size + 1);
    if (!index_html) {
        ESP_LOGE(TAG, "Failed to allocate memory for index.html");
        return ESP_ERR_NO_MEM;
    }

    FILE *fp = fopen(INDEX_HTML_PATH, "r");
    if (!fp) {
        ESP_LOGE(TAG, "Failed to open index.html");
        free(index_html);
        return ESP_FAIL;
    }

    fread(index_html, 1, st.st_size, fp);
    index_html[st.st_size] = '\0';
    fclose(fp);

    ESP_LOGI(TAG, "Loaded index.html (%d bytes)", (int)st.st_size);
    return ESP_OK;
}

// ---------------------------------------------------------
// HTTP GET handler for "/"
// ---------------------------------------------------------
static esp_err_t get_req_handler(httpd_req_t *req)
{
    if (!index_html) return ESP_FAIL;

    char *response_data = malloc(strlen(index_html) + 32);
    if (!response_data) return ESP_ERR_NO_MEM;

    snprintf(response_data, strlen(index_html) + 32, index_html, led_state ? "ON" : "OFF");
    esp_err_t ret = httpd_resp_send(req, response_data, HTTPD_RESP_USE_STRLEN);

    free(response_data);
    return ret;
}

// ---------------------------------------------------------
// WebSocket handler for "/ws"
// ---------------------------------------------------------
static esp_err_t ws_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "WebSocket handshake done");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(ws_pkt));

    // Get length of incoming frame
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) return ret;

    if (ws_pkt.len > 0) {
        ws_pkt.payload = malloc(ws_pkt.len + 1);
        if (!ws_pkt.payload) return ESP_ERR_NO_MEM;

        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            free(ws_pkt.payload);
            return ret;
        }
        ws_pkt.payload[ws_pkt.len] = '\0';

        ESP_LOGI(TAG, "Received WS message: %s", (char *)ws_pkt.payload);

        if (strcmp((char *)ws_pkt.payload, "toggle") == 0) {
            led_state = !led_state;
            gpio_set_level(LED_PIN, led_state);

            char send_buf[8];
            snprintf(send_buf, sizeof(send_buf), "%d", led_state);

            httpd_ws_frame_t send_pkt = {
                .payload = (uint8_t *)send_buf,
                .len = strlen(send_buf),
                .type = HTTPD_WS_TYPE_TEXT
            };
            httpd_ws_send_frame(req, &send_pkt);
        }

        free(ws_pkt.payload);
    }

    return ESP_OK;
}

// ---------------------------------------------------------
// Setup HTTP + WebSocket server
// ---------------------------------------------------------
static httpd_handle_t setup_websocket_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = get_req_handler,
        .user_ctx = NULL
    };

    httpd_uri_t uri_ws = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = ws_handler,
        .user_ctx = NULL
    };

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_ws);
    }

    return server;
}

// ---------------------------------------------------------
// Main setup function
// ---------------------------------------------------------
void web_server_setup(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Wi-Fi
    connect_wifi();
    if (!wifi_connect_status) {
        ESP_LOGE(TAG, "Wi-Fi not connected!");
        return;
    }

    // Initialize LED
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, led_state);

    // Serve webpage + WebSocket
    if (init_web_page_buffer() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to load index.html");
        return;
    }

    setup_websocket_server();
    ESP_LOGI(TAG, "ESP32 WebSocket Server running!");
}
