#pragma once

#include "pca9685.h"
#include <driver/i2c_master.h>

#define I2C_MASTER_SCL_IO   GPIO_NUM_9    /*!< GPIO number for I2C master clock */
#define I2C_MASTER_SDA_IO   GPIO_NUM_8    /*!< GPIO number for I2C master data */
#define I2C_MASTER_NUM      I2C_NUM_0 /*!< I2C port number for master */
#define I2C_ADDRESS_PCA9685_0 0x40  /*!< PCA9685 board 0 address */
#define I2C_ADDRESS_PCA9685_1 0x41  /*!< PCA9685 board 1 address */

namespace hexapod {

class Servo {
public:
    /** 
     * @brief Initialize PCA9685 boards and I2C bus. 
     * Must be called once before any Servo is used.
     */
    static void init();

    /**
     * @brief Construct a servo for a specific leg and joint.
     * @param legIndex Index of the leg (0-5)
     * @param jointIndex Index of the joint on that leg (0-2)
     * @param adjustAngle Optional mechanical adjustment in degrees
     * @param inverse Optional flag to invert motion direction
     * @param range Optional range limit in degrees
     */
    Servo(int legIndex, int jointIndex,
          float adjustAngle = 0.0f,
          bool inverse = false,
          float range = 60.0f);

    /** @brief Set the desired angle of the servo in degrees */
    void setAngle(float angle);

    /** @brief Get the last set angle of the servo */
    float getAngle() const;

    /** @brief Set an offset for the servo (in µs) */
    void setOffset(float offset);

    /** @brief Get the current offset */
    float getOffset() const;

private:
    int pwmIndex_;        /*!< PCA9685 channel index */
    bool inverse_;        /*!< Whether motion is inverted */
    float adjust_angle_;  /*!< Mechanical adjustment */
    float range_;         /*!< Max allowed angle */
    float angle_;         /*!< Last set angle */
    float offset_;        /*!< Pulse offset in µs */
};

} // namespace hexapod
