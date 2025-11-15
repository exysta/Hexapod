/***************************************************
  This is a library for the PCA9685 LED PWM Driver

  This chip is connected via I2C, 2 pins are required to interface. The PWM
  frequency is set for all pins, the PWM for each individually. The set PWM is
  active as long as the chip is powered.

  Written by Jonas Scharpf <jonas@brainelectronics.de>
  BSD license, all text above must be included in any redistribution

  ================================================================================

  This driver has been ported to use the esp-idf `driver/i2c_master.h` API.

  IMPORTANT: You must now initialize the driver using `pca9685_init()` after
  setting up the I2C master bus. This will configure the device handle used
  for all subsequent communications.

  ================================================================================

 ****************************************************/

#include "pca9685.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2c_master.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "PCA9685";

#define PWM_SERVO_LEN 64

const uint16_t pwmTable[256] = {
    0, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5,
    5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 12,
    12, 12, 13, 13, 13, 14, 14, 15, 15, 15, 16, 16, 17, 17, 18, 18,
    19, 19, 20, 21, 21, 22, 22, 23, 24, 24, 25, 26, 27, 27, 28, 29,
    30, 31, 31, 32, 33, 34, 35, 36, 37, 38, 39, 41, 42, 43, 44, 45,
    47, 48, 49, 51, 52, 54, 55, 57, 59, 60, 62, 64, 66, 68, 69, 71,
    74, 76, 78, 80, 82, 85, 87, 90, 92, 95, 98, 100, 103, 106, 109, 112,
    116, 119, 122, 126, 130, 133, 137, 141, 145, 149, 153, 158, 162, 167, 172, 177,
    182, 187, 193, 198, 204, 210, 216, 222, 228, 235, 241, 248, 255, 263, 270, 278,
    286, 294, 303, 311, 320, 330, 339, 349, 359, 369, 380, 391, 402, 413, 425, 437,
    450, 463, 476, 490, 504, 518, 533, 549, 564, 581, 597, 614, 632, 650, 669, 688,
    708, 728, 749, 771, 793, 816, 839, 863, 888, 913, 940, 967, 994, 1023, 1052, 1082,
    1114, 1146, 1178, 1212, 1247, 1283, 1320, 1358, 1397, 1437, 1478, 1520, 1564, 1609, 1655, 1703,
    1752, 1802, 1854, 1907, 1962, 2018, 2076, 2135, 2197, 2260, 2325, 2391, 2460, 2531, 2603, 2678,
    2755, 2834, 2916, 2999, 3085, 3174, 3265, 3359, 3455, 3555, 3657, 3762, 3870, 3981, 4095
};

const uint16_t pwmTableServo[64] = {
    90, 96, 103, 109, 116, 122, 129, 135,
    142, 148, 155, 161, 168, 174, 181, 187,
    194, 200, 207, 213, 220, 226, 233, 239,
    246, 252, 259, 265, 272, 278, 285, 291,
    298, 304, 311, 317, 324, 330, 337, 343,
    350, 356, 363, 369, 376, 382, 389, 395,
    402, 408, 415, 421, 428, 434, 441, 447,
    454, 460, 467, 473, 480, 486, 493, 499,
    506, 512
};

/**
 * @brief Initialize the PCA9685 driver
 */
esp_err_t pca9685_init(pca9685_t *pca, i2c_master_bus_handle_t bus_handle, uint8_t device_address)
{
    if (!pca) return ESP_ERR_INVALID_ARG;

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = device_address,
        .scl_speed_hz = 400000,
    };

    return i2c_master_bus_add_device(bus_handle, &dev_cfg, &pca->device_handle);
}

esp_err_t pca9685_deinit(pca9685_t *pca)
{
    if (!pca || pca->device_handle == NULL)
        return ESP_OK;

    esp_err_t ret = i2c_master_bus_rm_device(pca->device_handle);
    if (ret == ESP_OK) pca->device_handle = NULL;
    return ret;
}

// --- Static Helper Functions ---

static esp_err_t i2c_write(pca9685_t *pca, uint8_t regaddr, const uint8_t *data, size_t len)
{
    if (pca->device_handle == NULL) {
        ESP_LOGE(TAG, "Driver not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    uint8_t write_buf[len + 1];
    write_buf[0] = regaddr;
    memcpy(write_buf + 1, data, len);
    return i2c_master_transmit(pca->device_handle, write_buf, sizeof(write_buf), -1);
}

static esp_err_t i2c_read(pca9685_t *pca, uint8_t regaddr, uint8_t *data, size_t len)
{
    if (pca->device_handle == NULL) {
        ESP_LOGE(TAG, "Driver not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    return i2c_master_transmit_receive(pca->device_handle, &regaddr, 1, data, len, -1);
}

// --- Public API Functions ---

esp_err_t pca9685_reset(pca9685_t *pca)
{
    const uint8_t data = 0x80;
    esp_err_t ret = i2c_write(pca, MODE1, &data, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    return ret;
}

esp_err_t pca9685_set_frequency(pca9685_t *pca, uint16_t freq)
{
    esp_err_t ret;
    uint8_t data;

    ret = i2c_read(pca, MODE1, &data, 1);
    if (ret != ESP_OK) return ret;

    uint8_t old_mode = data;
    uint8_t new_mode = (old_mode & 0x7F) | 0x10;
    ret = i2c_write(pca, MODE1, &new_mode, 1);
    if (ret != ESP_OK) return ret;

    uint8_t prescale_val = round((float)CLOCK_FREQ / (4096.0f * (float)freq)) - 1;
    ret = i2c_write(pca, PRE_SCALE, &prescale_val, 1);
    if (ret != ESP_OK) return ret;

    ret = i2c_write(pca, MODE1, &old_mode, 1);
    if (ret != ESP_OK) return ret;

    vTaskDelay(pdMS_TO_TICKS(5));
    data = old_mode | 0xa0;
    return i2c_write(pca, MODE1, &data, 1);
}

esp_err_t pca9685_set_pwm(pca9685_t *pca, uint8_t num, uint16_t on, uint16_t off)
{
    if (num > 15) return ESP_ERR_INVALID_ARG;

    uint8_t buffer[4];
    buffer[0] = on & 0xFF;
    buffer[1] = on >> 8;
    buffer[2] = off & 0xFF;
    buffer[3] = off >> 8;

    uint8_t reg_addr = LED0_ON_L + (LED_MULTIPLYER * num);
    return i2c_write(pca, reg_addr, buffer, 4);
}

esp_err_t pca9685_get_pwm(pca9685_t *pca, uint8_t num, uint16_t* dataOn, uint16_t* dataOff)
{
    if (num > 15 || !dataOn || !dataOff) return ESP_ERR_INVALID_ARG;

    uint8_t buffer[4];
    uint8_t reg_addr = LED0_ON_L + (LED_MULTIPLYER * num);
    esp_err_t ret = i2c_read(pca, reg_addr, buffer, 4);

    if (ret == ESP_OK) {
        *dataOn = (buffer[1] << 8) | buffer[0];
        *dataOff = (buffer[3] << 8) | buffer[2];
    }
    return ret;
}

esp_err_t pca9685_turn_all_off(pca9685_t *pca)
{
    uint8_t buffer[4] = {0x00, 0x00, 0x00, 0x10};
    return i2c_write(pca, ALLLED_ON_L, buffer, 4);
}

esp_err_t pca9685_fade_pin_up_down(pca9685_t *pca, uint8_t pin)
{
    esp_err_t ret = ESP_OK;
    for (uint16_t i = 0; i < PWM_SERVO_LEN-1; i++) {
        ret = pca9685_set_pwm(pca, pin, 0, pwmTableServo[i]);
        if (ret != ESP_OK) return ret;
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    for (uint16_t i = 0; i < PWM_SERVO_LEN-1; i++) {
        ret = pca9685_set_pwm(pca, pin, 0, pwmTableServo[PWM_SERVO_LEN-1 - i]);
        if (ret != ESP_OK) return ret;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    return pca9685_set_pwm(pca, pin, 0, 4096);
}

esp_err_t pca9685_fade_all_up_down(pca9685_t *pca)
{
    esp_err_t ret = ESP_OK;
    for (uint8_t pin = 0; pin < 16; pin++) {
        ESP_LOGI(TAG, "Fading pin %d", pin);
        ret = pca9685_fade_pin_up_down(pca, pin);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to fade pin %d", pin);
            return ret;
        }
    }
    return ret;
}

void pca9685_disp_buf(uint16_t* buf, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        printf("%04X ", buf[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}
