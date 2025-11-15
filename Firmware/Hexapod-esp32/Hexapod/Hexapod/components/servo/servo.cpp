#pragma once
#include <stdio.h>
#include "pca9685.h"
#include "sdkconfig.h"
#include <driver/i2c_master.h>
#include <esp_log.h>
#include <mutex>
#include "servo.h"

namespace hexapod {

namespace {

constexpr int kFrequency = 50;          // Servo frequency (Hz)
constexpr int kTickUs = 1000000 / (kFrequency * 4096); // microseconds per PCA9685 tick
constexpr int kServoMiddle = 1500;      // Center pulse width in µs
constexpr int kServoMin = 500;          // Min pulse width
constexpr int kServoMax = 2500;         // Max pulse width
constexpr int kServoRange = kServoMax - kServoMiddle;

static const char* TAG = "SERVO";

// PCA9685 devices
pca9685_t pca9685_left;
pca9685_t pca9685_right;

// Thread-safe initialization
bool pwmInited = false;
std::mutex pwmInitMutex;
i2c_master_bus_handle_t bus_handle;

// Leg-to-PWM mapping (legIndex, jointIndex) -> PWM channel
constexpr int kLegs = 6;
constexpr int kJoints = 3;
constexpr int kTotalServos = kLegs * kJoints;

constexpr int hexapodToPwm[kLegs][kJoints] = {
    {5, 6, 7},       // Leg 0
    {2, 3, 4},       // Leg 1
    {8, 9, 10},      // Leg 2
    {24, 25, 26},    // Leg 3 (16+8+0..2)
    {18, 19, 20},    // Leg 4 (16+2+0..2)
    {21, 22, 23}     // Leg 5 (16+5+0..2)
};

// Map PWM channel back to legIndex/jointIndex (flattened)
inline int pwm2Leg(int pwm) {
    for (int leg = 0; leg < kLegs; ++leg) {
        for (int joint = 0; joint < kJoints; ++joint) {
            if (hexapodToPwm[leg][joint] == pwm)
                return leg;
        }
    }
    return -1;
}

// Map PWM channel back to joint index
inline int pwm2Joint(int pwm) {
    for (int leg = 0; leg < kLegs; ++leg) {
        for (int joint = 0; joint < kJoints; ++joint) {
            if (hexapodToPwm[leg][joint] == pwm)
                return joint;
        }
    }
    return -1;
}

// Initialize I2C bus
i2c_master_bus_handle_t i2c_init() {
    ESP_LOGI(TAG, "Initializing I2C Master Bus...");

    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .flags.enable_internal_pullup = 1,
    };

    i2c_master_bus_handle_t handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &handle));
    return handle;
}

// Initialize PCA9685 boards
void initPWM() {
    std::lock_guard<std::mutex> lock(pwmInitMutex);
    if (pwmInited) return;

    bus_handle = i2c_init();

    ESP_ERROR_CHECK(pca9685_init(&pca9685_left, bus_handle, I2C_ADDRESS_PCA9685_0));
    ESP_ERROR_CHECK(pca9685_init(&pca9685_right, bus_handle, I2C_ADDRESS_PCA9685_1));

    ESP_ERROR_CHECK(pca9685_reset(&pca9685_left));
    ESP_ERROR_CHECK(pca9685_reset(&pca9685_right));

    ESP_ERROR_CHECK(pca9685_set_frequency(&pca9685_left, kFrequency));
    ESP_ERROR_CHECK(pca9685_set_frequency(&pca9685_right, kFrequency));

    pwmInited = true;
}

} // namespace

class Servo {
public:
    Servo(int legIndex, int jointIndex, float adjustAngle = 0.0f, bool inverse = false, float range = 60.0f)
        : adjust_angle_(adjustAngle),
          inverse_(inverse),
          range_(range),
          angle_(0),
          offset_(0)
    {
        pwmIndex_ = hexapodToPwm[legIndex][jointIndex];
    }

    void init() {
        initPWM();
    }

    void setAngle(float angle) {
        // Apply adjustment and inversion
        float effectiveAngle = inverse_ ? -(angle - adjust_angle_) : (angle - adjust_angle_);

        // Clip to allowed range
        if (effectiveAngle > range_) {
            ESP_LOGI(TAG, "Angle exceeded max[%d]=%.2f", pwm2Leg(pwmIndex_), angle);
            effectiveAngle = range_;
        } else if (effectiveAngle < -range_) {
            ESP_LOGI(TAG, "Angle exceeded min[%d]=%.2f", pwm2Leg(pwmIndex_), angle);
            effectiveAngle = -range_;
        }

        angle_ = angle; // store requested angle

        // Determine board and channel
        pca9685_t* pca = (pwmIndex_ < 16) ? &pca9685_right : &pca9685_left;
        int idx = (pwmIndex_ < 16) ? pwmIndex_ : pwmIndex_ - 16;

        // Compute pulse width in µs
        float pulseUs = kServoMiddle + effectiveAngle * (kServoRange / 90.0f) + offset_;
        if (pulseUs > kServoMax) pulseUs = kServoMax;
        if (pulseUs < kServoMin) pulseUs = kServoMin;

        // Convert to PCA9685 ticks
        int ticks = static_cast<int>(pulseUs / kTickUs);

        ESP_ERROR_CHECK(pca9685_set_pwm(pca, idx, 0, ticks));

        ESP_LOGD(TAG, "Servo[%d] angle=%.2f µs=%.2f ticks=%d", pwm2Leg(pwmIndex_), angle, pulseUs, ticks);
    }

    float getAngle() const { return angle_; }

    void setOffset(float offset) { offset_ = offset; }
    float getOffset() const { return offset_; }

private:
    int pwmIndex_;
    bool inverse_;
    float adjust_angle_;
    float range_;
    float angle_;
    float offset_;
};

} // namespace hexapod
