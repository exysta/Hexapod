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
#define MODE1           0x00    /*!< Mode register 1 */
#define MODE2           0x01    /*!< Mode register 2 */
#define SUBADR1         0x02    /*!< I2C-bus subaddress 1 */
#define SUBADR2         0x03    /*!< I2C-bus subaddress 2 */
#define SUBADR3         0x04    /*!< I2C-bus subaddress 3 */
#define ALLCALLADR      0x05    /*!< LED All Call I2C-bus address */
#define LED0_ON_L       0x06    /*!< LED0 output and brightness control byte 0 */
#define LED0_ON_H       0x07    /*!< LED0 output and brightness control byte 1 */
#define LED0_OFF_L      0x08    /*!< LED0 output and brightness control byte 2 */
#define LED0_OFF_H      0x09    /*!< LED0 output and brightness control byte 3 */
#define LED_MULTIPLYER  4       /*!< For the other 15 channels */
#define ALLLED_ON_L     0xFA    /*!< load all the LEDn_ON registers, byte 0 */
#define ALLLED_ON_H     0xFB    /*!< load all the LEDn_ON registers, byte 1 */
#define ALLLED_OFF_L    0xFC    /*!< load all the LEDn_OFF registers, byte 0 */
#define ALLLED_OFF_H    0xFD    /*!< load all the LEDn_OFF registers, byte 1 */
#define PRE_SCALE       0xFE    /*!< prescaler for output frequency */
#define CLOCK_FREQ      25000000.0  /*!< 25MHz default osc clock */

typedef struct {
    i2c_master_dev_handle_t device_handle;
    uint8_t address;
} pca9685_t;

/**
 * @brief Initialize the PCA9685 driver.
 *
 * This function must be called once after the I2C bus is initialized. It adds the
 * PCA9685 device to the bus and gets a handle for future communications.
 *
 * @param[in] bus_handle Handle for the I2C master bus.
 * @param[in] device_address The 7-bit I2C address of the PCA9685.
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t pca9685_init(pca9685_t *pca,i2c_master_bus_handle_t bus_handle, uint8_t device_address);

/**
 * @brief Deinitialize the PCA9685 driver.
 *
 * Removes the PCA9685 device from the I2C bus and frees resources.
 *
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t pca9685_deinit(void);

/**
 * @brief Send a software reset to the PCA9685 chip.
 */
esp_err_t resetPCA9685(void);

/**
 * @brief Set the PWM frequency for all channels.
 *
 * @param[in] freq The desired frequency in Hz (typically 40-1000).
 */
esp_err_t setFrequencyPCA9685(uint16_t freq);

/**
 * @brief Turn all 16 channels fully off.
 */
esp_err_t turnAllOff(void);

/**
 * @brief Set the PWM duty cycle for a single channel.
 *
 * @param[in] num The channel number (0-15).
 * @param[in] on The tick (from 0-4095) when the signal should go high.
 * @param[in] off The tick (from 0-4095) when the signal should go low.
 */
esp_err_t setPWM(uint8_t num, uint16_t on, uint16_t off);

/**
 * @brief Get the current PWM on/off values for a single channel.
 *
 * @param[in] num The channel number (0-15).
 * @param[out] dataOn Pointer to a uint16_t to store the ON value.
 * @param[out] dataOff Pointer to a uint16_t to store the OFF value.
 */
esp_err_t getPWM(uint8_t num, uint16_t* dataOn, uint16_t* dataOff);

/**
 * @brief Demonstration function to fade a single pin up and down.
 *
 * @param[in] pin The pin number (0-15) to fade.
 */
esp_err_t fade_pin_up_down(uint8_t pin);

/**
 * @brief Demonstration function to fade all pins sequentially up and down.
 */
esp_err_t fade_all_up_down(void);

/**
 * @brief Utility function to print a buffer of 16-bit values.
 *
 * @param buf Pointer to the buffer.
 * @param len The number of elements in the buffer.
 */
void disp_buf(uint16_t* buf, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* PCA9685_DRIVER_H */