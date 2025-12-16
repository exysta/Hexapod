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
#include "cJSON.h"

// Include your custom headers
#include "connect_wifi.h"
#include "web-server.h"
#include "led_strip.h"

// --- LED CONFIG (Kept from your original code) ---
led_strip_handle_t strip;
led_strip_config_t strip_config = {
    .max_leds = 1,
    .strip_gpio_num = 48,
    .led_model = LED_MODEL_WS2812
};
led_strip_rmt_config_t rmt_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT,
    .resolution_hz = 10000000,
    .mem_block_symbols = 64,
    .flags.with_dma = 1
};

static const char *TAG = "RobotServer";
static httpd_handle_t server = NULL;

#define WEB_CONTROLLER_HTML_PATH "/spiffs/web_controller.html"
#define CALIBRATION_HTML_PATH "/spiffs/calibration.html"

static char *web_controller_html = NULL;
static char *calibration_html = NULL;

// ---------------------------------------------------------
// Helper: Load a file from SPIFFS into a char buffer
// ---------------------------------------------------------
static esp_err_t load_file_to_memory(const char *path, char **buffer)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        ESP_LOGE(TAG, "%s not found", path);
        return ESP_FAIL;
    }

    *buffer = malloc(st.st_size + 1);
    if (*buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for %s", path);
        return ESP_ERR_NO_MEM;
    }

    FILE *fp = fopen(path, "r");
    if (!fp) {
        ESP_LOGE(TAG, "Failed to open %s", path);
        free(*buffer);
        *buffer = NULL;
        return ESP_FAIL;
    }

    fread(*buffer, 1, st.st_size, fp);
    (*buffer)[st.st_size] = '\0'; // Null-terminate string
    fclose(fp);

    ESP_LOGI(TAG, "Loaded %s (%ld bytes)", path, st.st_size);
    return ESP_OK;
}

// ---------------------------------------------------------
// Load webpages from SPIFFS
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

    // Load Index Page
    if (load_file_to_memory(WEB_CONTROLLER_HTML_PATH, &web_controller_html) != ESP_OK) {
        return ESP_FAIL;
    }

    // Load Calibration Page
    if (load_file_to_memory(CALIBRATION_HTML_PATH, &calibration_html) != ESP_OK) {
        ESP_LOGW(TAG, "Calibration file missing, skipping...");
        // We don't return ESP_FAIL here so the rest of the server still works
    }
    return ESP_OK;
}

// ---------------------------------------------------------
// HTTP GET handler for "/"
// ---------------------------------------------------------
static esp_err_t get_req_handler(httpd_req_t *req)
{
    if (!web_controller_html) return ESP_FAIL;
    // The new HTML doesn't use the %s format specifier for LED state anymore, 
    // so we just send the raw HTML.
    httpd_resp_send(req, web_controller_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// ---------------------------------------------------------
// HTTP GET handler for "/calibration" (Placeholder)
// ---------------------------------------------------------
static esp_err_t calibration_req_handler(httpd_req_t *req)
{
    if (!calibration_html) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    httpd_resp_send(req, calibration_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// ---------------------------------------------------------
// WebSocket handler for "/cmd"
// ---------------------------------------------------------
static esp_err_t cmd_ws_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "WebSocket handshake done");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(ws_pkt));

    // Get length
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

        // ESP_LOGI(TAG, "Raw received: %s", (char *)ws_pkt.payload);

        // --- JSON PARSING ---
        cJSON *root = cJSON_Parse((char *)ws_pkt.payload);
        if (root) {
            // 1. Check for Movement Command
            cJSON *movement = cJSON_GetObjectItem(root, "movementMode");
            if (movement) {
                int cmd = movement->valueint;
                ESP_LOGI(TAG, "Movement Command Received: %d", cmd);
                
                // Example: Control LED based on command (Visual feedback)
                if (cmd == 1) { // Standby
                     led_strip_set_pixel(strip, 0, 0, 0, 0); // Off
                } else {
                     led_strip_set_pixel(strip, 0, 0, 255, 0); // Green for active
                }
                led_strip_refresh(strip);

                // TODO: Insert your robot motor control logic here based on 'cmd'
                // 1=Forward, 2=Run, 3=Back, 4=Left, 5=Right, etc.
            }

            // 2. Check for Speed Command
            cJSON *speed = cJSON_GetObjectItem(root, "speed");
            if (speed) {
                double spd = speed->valuedouble;
                ESP_LOGI(TAG, "Speed Set: %.2f", spd);
                // TODO: Update robot global speed variable here
            }
            // 3. Check for CALIBRATION COMMANDS
            cJSON *cal = cJSON_GetObjectItem(root, "cal_action");
            if (cal) {
                char *action = cal->valuestring; // It's a string like "save" or "start"
                ESP_LOGI(TAG, "Calibration Action: %s", action);

                if (strcmp(action, "save") == 0) {
                    ESP_LOGI(TAG, "Saving calibration to NVS...");
                    // Call your save function here
                } 
                else if (strcmp(action, "start") == 0) {
                    ESP_LOGI(TAG, "Starting calibration routine...");
                    // Call your calibration start function here
                }
            }

            cJSON_Delete(root);
        } else {
            ESP_LOGW(TAG, "Failed to parse JSON");
        }
        
        // --- OPTIONAL: Send Battery Status Back ---
        // The HTML expects a message like: {"raw": 85}
        // This is just a simulation.
        const char *battery_json = "{\"raw\": 95}"; 
        httpd_ws_frame_t send_pkt = {
            .payload = (uint8_t *)battery_json,
            .len = strlen(battery_json),
            .type = HTTPD_WS_TYPE_TEXT
        };
        httpd_ws_send_frame(req, &send_pkt);

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
    
    // Increase URI match length slightly if needed, default is usually fine
    config.max_uri_handlers = 12; 

    // URI: / (Index)
    httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = get_req_handler,
        .user_ctx = NULL
    };

    // URI: /calibration
    httpd_uri_t uri_cal = {
        .uri = "/calibration",
        .method = HTTP_GET,
        .handler = calibration_req_handler,
        .user_ctx = NULL
    };

    // URI: /cmd (WebSocket) -> Note: changed from /ws to /cmd to match HTML
    httpd_uri_t uri_ws = {
        .uri = "/cmd",
        .method = HTTP_GET,
        .handler = cmd_ws_handler,
        .user_ctx = NULL,
        .is_websocket = true
    };

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_cal);
        httpd_register_uri_handler(server, &uri_ws);
        ESP_LOGI(TAG, "Server started on port 80");
    }

    return server;
}

// ---------------------------------------------------------
// Main setup function
// ---------------------------------------------------------
void web_server_setup(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    connect_wifi();
    if (!wifi_connect_status) {
        ESP_LOGE(TAG, "Wi-Fi not connected!");
        return;
    }

    ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LED strip initialization failed");
    }

    if (init_web_page_buffer() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to load web_controller.html. Ensure SPIFFS is uploaded.");
        return;
    }

    setup_websocket_server();
}