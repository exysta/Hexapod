/***************************************************
  This is a library for the PCA9685 LED PWM Driver

  This chip is connected via I2C, 2 pins are required to interface. The PWM
  frequency is set for all pins, the PWM for each individually. The set PWM is
  active as long as the chip is powered.

  Written by Jonas Scharpf <jonas@brainelectronics.de>
  BSD license, all text above must be included in any redistribution

  ================================================================================

  This driver has been ported to use the modern esp-idf `driver/i2c_master.h` API.

  IMPORTANT: You must initialize the driver using `pca9685_init()` after setting
  up the I2C master bus. This function configures the device for all subsequent
  communications.
  ================================================================================

 ****************************************************/

#ifndef PCA9685_DRIVER_H
#define PCA9685_DRIVER_H

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

// --- Register Definitions ---
#define MODE1           0x00
#define MODE2           0x01
#define SUBADR1         0x02
#define SUBADR2         0x03
#define SUBADR3         0x04
#define ALLCALLADR      0x05
#define LED0_ON_L       0x06
#define LED0_ON_H       0x07
#define LED0_OFF_L      0x08
#define LED0_OFF_H      0x09
#define LED_MULTIPLYER  4
#define ALLLED_ON_L     0xFA
#define ALLLED_ON_H     0xFB
#define ALLLED_OFF_L    0xFC
#define ALLLED_OFF_H    0xFD
#define PRE_SCALE       0xFE
#define CLOCK_FREQ      25000000.0

typedef struct {
    i2c_master_dev_handle_t device_handle;
    uint8_t address;
} pca9685_t;

/**
 * @brief Initialize the PCA9685 driver.
 */
esp_err_t pca9685_init(pca9685_t *pca, i2c_master_bus_handle_t bus_handle, uint8_t device_address);

/**
 * @brief Deinitialize the PCA9685 driver.
 */
esp_err_t pca9685_deinit(pca9685_t *pca);

/**
 * @brief Send a software reset to the PCA9685 chip.
 */
esp_err_t pca9685_reset(pca9685_t *pca);

/**
 * @brief Set the PWM frequency for all channels.
 */
esp_err_t pca9685_set_frequency(pca9685_t *pca, uint16_t freq);

/**
 * @brief Turn all 16 channels fully off.
 */
esp_err_t pca9685_turn_all_off(pca9685_t *pca);

/**
 * @brief Set the PWM duty cycle for a single channel.
 */
esp_err_t pca9685_set_pwm(pca9685_t *pca, uint8_t num, uint16_t on, uint16_t off);

/**
 * @brief Get the current PWM on/off values for a single channel.
 */
esp_err_t pca9685_get_pwm(pca9685_t *pca, uint8_t num, uint16_t* dataOn, uint16_t* dataOff);

/**
 * @brief Demonstration function to fade a single pin up and down.
 */
esp_err_t pca9685_fade_pin_up_down(pca9685_t *pca, uint8_t pin);

/**
 * @brief Demonstration function to fade all pins sequentially up and down.
 */
esp_err_t pca9685_fade_all_up_down(pca9685_t *pca);

/**
 * @brief Utility function to print a buffer of 16-bit values.
 */
void pca9685_disp_buf(uint16_t* buf, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* PCA9685_DRIVER_H */
