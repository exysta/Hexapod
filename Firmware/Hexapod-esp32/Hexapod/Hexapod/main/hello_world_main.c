/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <driver/i2c_master.h>
#include <esp_log.h>
#include <math.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "pca9685.h"

// GPIO configuration
#define I2C_EXAMPLE_MASTER_SCL_IO   9    /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO   8    /*!< gpio number for I2C master data  */
#define I2C_EXAMPLE_MASTER_NUM      I2C_NUM_0   /*!< I2C port number for master dev */
#define I2C_ADDRESS                 0x40    /*!< slave address for PCA9685 */

static const char *TAG = "PCA9685_MAIN";

/**
 * @brief Initialize the I2C master bus
 * 
 * @return i2c_master_bus_handle_t Handle to the created bus
 */
static i2c_master_bus_handle_t i2c_example_master_init(void)
{
    ESP_LOGI(TAG, "Initializing I2C Master Bus...");

    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_EXAMPLE_MASTER_NUM,
        .sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO,
        .scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .flags.enable_internal_pullup = 1, // Enable internal pullups
    };

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));
    
    return bus_handle;
}

void task_PCA9685(void *ignore)
{
    printf("Executing on core %d\n", xPortGetCoreID());

    esp_err_t ret;

    // 1. Initialize the I2C Bus
    i2c_master_bus_handle_t bus_handle = i2c_example_master_init();

    // 2. Initialize the PCA9685 Driver with the bus handle
    // This replaces set_pca9685_adress()
    ESP_LOGI(TAG, "Initializing PCA9685 Driver...");
    ESP_ERROR_CHECK(pca9685_init(bus_handle, I2C_ADDRESS));

    // 3. Configure the chip
    resetPCA9685();
    setFrequencyPCA9685(50);
    turnAllOff();

    printf("Finished setup, entering loop now\n");

    while(1)
    {
        // fade up and down each pin with static logarithmic table
        // see Weber Fechner Law
        printf("fade up down all\n");
        ret = fade_pin_up_down(0);

        if(ret == ESP_ERR_TIMEOUT)
        {
            ESP_LOGE(TAG, "I2C timeout");
        }
        else if(ret != ESP_OK)
        {
            ESP_LOGE(TAG, "I2C Error: %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(5000));

        /*
        // Example: Blink all pins starting from 0
        // This section is commented out but updated to be compatible with the new driver if enabled.
        
        // for (uint8_t pin = 0; pin < 16; pin++)
        // {
        //     printf("Turn LED %d on\n", pin);
        //     setPWM(pin, 0, 4096); // Full ON (On=0, Off=4096 is actually invalid for fully on, usually On=4096, Off=0 or special bits)
                                     // Note: Previous code had setPWM(pin, 4096, 0); Check datasheet for full ON bit usage if needed, 
                                     // or use setPWM(pin, 0, 4095); 

        //     vTaskDelay(pdMS_TO_TICKS(100));

        //     printf("Turn LED %d off\n", pin);
        //     setPWM(pin, 0, 0); // Off
        // }
        */

        /*
        // Read back example
        // led turn on and 100ms off, starting from pin 0...15
        // read back the set value of each pin
        for (uint8_t pin = 0; pin < 16; pin++)
        {
            // printf("Turn LED %d on to H(%d) L(%d)\n", pin, 4095-pin, 0);
            setPWM(pin, 4095-pin, 0);

            uint16_t myDataOn;
            uint16_t myDataOff;

            // Note: getPWMDetail was removed in the new driver as getPWM reads everything efficiently
            ret = getPWM(pin, &myDataOn, &myDataOff);

            if(ret == ESP_OK)
            {
                printf("Read back from device pin %d: On(%d) Off(%d)\n", pin, myDataOn, myDataOff);
            }
            else
            {
                printf("Failed to read\n");
            }

            vTaskDelay(pdMS_TO_TICKS(500));

            // printf("Turn LED %d off\n", pin);
            setPWM(pin, 0, 4096); // Full off
        }
        */
    }

    // Should technically never reach here, but good practice
    pca9685_deinit();
    vTaskDelete(NULL);
}

void app_main()
{
    xTaskCreate(task_PCA9685, "task_PCA9685", 4096, NULL, 10, NULL);
}