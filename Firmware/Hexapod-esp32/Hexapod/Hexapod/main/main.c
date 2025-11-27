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
#include "web-server.h"

#include "led_strip.h" // to remove later
static const char *TAG = "MAIN";


// to remove
// List of most common GPIOs used for WS2812 on ESP32-S3 boards
// 48: Standard DevKitC-1
// 38: DevKitC-1 v1.1 / S3-BOX
// 47: Some clones
// 21: S3-Zero / S3-Mini
// 39-42, 45-46: Other common variations
int plausible_gpios[] = {48, 38, 47, 21, 39, 40, 41, 42, 45, 46, 1, 2, 10, 15};

void test_pin(int gpio_num)
{
    led_strip_handle_t led_strip = NULL;

    // 1. Configure the LED strip for the current GPIO
    led_strip_config_t strip_config = {
        .strip_gpio_num = gpio_num,
        .max_leds = 1, 
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };

    // 2. Init the driver
    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
    
    if (err == ESP_OK) {
        ESP_LOGW(TAG, ">>> Testing GPIO %d <<<", gpio_num);
        
        // 3. Set Color to RED (Green=0, Red=100, Blue=0)
        led_strip_set_pixel(led_strip, 0, 0, 100, 0);
        led_strip_refresh(led_strip);

        // 4. Wait so you can see it
        vTaskDelay(500 / portTICK_PERIOD_MS);

        // 5. Turn off
        led_strip_clear(led_strip);
        
        // 6. IMPORTANT: Delete the driver to free up the RMT channel for the next pin
        led_strip_del(led_strip);
    } else {
        ESP_LOGE(TAG, "Could not init GPIO %d (might be input-only or busy)", gpio_num);
    }
}

// to remove



void task_PCA9685(void *ignore)
{
    printf("Executing on core %d\n", xPortGetCoreID());

    // esp_err_t ret;

    // // 1. Initialize the I2C Bus
    // i2c_master_bus_handle_t bus_handle = i2c_example_master_init();

    // // 2. Initialize the PCA9685 Driver with the bus handle
    // // This replaces set_pca9685_adress()
    // ESP_LOGI(TAG, "Initializing PCA9685 Driver...");
    // ESP_ERROR_CHECK(pca9685_init(bus_handle, I2C_ADDRESS));

    // // 3. Configure the chip
    // resetPCA9685();
    // setFrequencyPCA9685(50);
    // turnAllOff();

    // printf("Finished setup, entering loop now\n");

    // while(1)
    // {
    //     // fade up and down each pin with static logarithmic table
    //     // see Weber Fechner Law
    //     printf("fade up down all\n");
    //     ret = fade_pin_up_down(0);

    //     if(ret == ESP_ERR_TIMEOUT)
    //     {
    //         ESP_LOGE(TAG, "I2C timeout");
    //     }
    //     else if(ret != ESP_OK)
    //     {
    //         ESP_LOGE(TAG, "I2C Error: %s", esp_err_to_name(ret));
    //     }

    //     vTaskDelay(pdMS_TO_TICKS(5000));

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
    // }

    // Should technically never reach here, but good practice
    // pca9685_deinit();
    // vTaskDelete(NULL);
}

void task_web_server(void* ignore)
{
    printf("Executing WebServer Task");

    vTaskDelete(NULL);
}

void app_main()
{
    // ESP_LOGI(TAG, "Starting Pin Hunter...");
    // ESP_LOGI(TAG, "Watch the LED. When it lights up, check the log!");

    // while (1) {
    //     // Loop through our list of candidates
    //     int num_candidates = sizeof(plausible_gpios) / sizeof(plausible_gpios[0]);
        
    //     for (int i = 0; i < num_candidates; i++) {
    //         // test_pin(plausible_gpios[i]);
    //         test_pin(48);

    //         // Small delay between pins
    //         vTaskDelay(100 / portTICK_PERIOD_MS);
    //     }

    //     ESP_LOGI(TAG, "Restarting loop...");
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    web_server_setup();

    // xTaskCreate(task_PCA9685, "task_PCA9685", 4096, NULL, 10, NULL);

}