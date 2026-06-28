#pragma once
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <thread>

#ifdef USE_HARDWARE_SENSOR
#include "SPIDriver.hpp"
#endif

struct RawImuData
{
    float accelX = 0, accelY = 0, accelZ = 1;
    float gyroX = 0, gyroY = 0, gyroZ = 0;
};

struct TiltReading
{
    float tiltX = 0;
    float tiltY = 0;
};

class GyroscopeReader
{
public:

    TiltReading read();

private:

    void initialiseHardware();

    RawImuData readRawImu();

    float rollAngle_ = 0.0f;
    float pitchAngle_ = 0.0f;

    std::chrono::steady_clock::time_point lastReadTime_ = std::chrono::steady_clock::now();

    bool initialised_ = false;

    static constexpr float MAX_TILT_DEGREES = 45.0f;

    static constexpr float GYRO_WEIGHT = 0.999f;

    static constexpr float ACTIVATE_THRESHOLD = 0.25f;
    static constexpr float RELEASE_THRESHOLD = 0.10f;

    bool hysteresisActiveX_ = false;
    bool hysteresisActiveY_ = false;

    static float clamp(float value, float lowerBound, float upperBound)
    {
        return value < lowerBound   ? lowerBound
               : value > upperBound ? upperBound
                                    : value;
    }

    float applyHysteresis(float value, bool &active)
    {
        if (active)
        {
            if (std::abs(value) < RELEASE_THRESHOLD)
            {
                active = false;
                return 0.0f;
            }
        }
        else
        {
            if (std::abs(value) < ACTIVATE_THRESHOLD)
            {
                return 0.0f;
            }
            active = true;
        }
        return value;
    }

#ifdef USE_HARDWARE_SENSOR
    std::unique_ptr<SPIDriver> spiDriver_;
#endif
};

inline TiltReading GyroscopeReader::read()
{
    if (!initialised_)
    {
        initialiseHardware();
        initialised_ = true;
    }

    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - lastReadTime_).count();
    lastReadTime_ = currentTime;

    RawImuData rawSensorData = readRawImu();

    float accelRollAngle = -std::atan2(rawSensorData.accelY,
                                       std::sqrt(rawSensorData.accelX * rawSensorData.accelX + rawSensorData.accelZ * rawSensorData.accelZ)) *
                           (180.0f / M_PI);

    float accelPitchAngle = std::atan2(rawSensorData.accelX,
                                       std::sqrt(rawSensorData.accelY * rawSensorData.accelY + rawSensorData.accelZ * rawSensorData.accelZ)) *
                            (180.0f / M_PI);

    rollAngle_ = GYRO_WEIGHT * (rollAngle_ - rawSensorData.gyroX * deltaTime) + (1.0f - GYRO_WEIGHT) * accelRollAngle;

    pitchAngle_ = GYRO_WEIGHT * (pitchAngle_ - rawSensorData.gyroY * deltaTime) + (1.0f - GYRO_WEIGHT) * accelPitchAngle;

    float normalizedX = clamp(rollAngle_ / MAX_TILT_DEGREES, -1.0f, 1.0f);
    float normalizedY = clamp(-pitchAngle_ / MAX_TILT_DEGREES, -1.0f, 1.0f);

    TiltReading tiltOutput;
    tiltOutput.tiltX = applyHysteresis(normalizedX, hysteresisActiveX_);
    tiltOutput.tiltY = applyHysteresis(normalizedY, hysteresisActiveY_);

    if (!std::isfinite(tiltOutput.tiltX) || !std::isfinite(tiltOutput.tiltY))
    {
        rollAngle_ = 0.0f;
        pitchAngle_ = 0.0f;
        return TiltReading{};
    }
    return tiltOutput;
}

#ifdef USE_HARDWARE_SENSOR

inline void GyroscopeReader::initialiseHardware()
{
    spiDriver_ = std::make_unique<SPIDriver>("/dev/spidev0.0");

    uint8_t command = 0x11;
    spiDriver_->write(0x7E, &command, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    command = 0x15;
    spiDriver_->write(0x7E, &command, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(80));

    std::cout << "[GyroscopeReader] BMI160 initialised on /dev/spidev0.0\n";
}

inline RawImuData GyroscopeReader::readRawImu()
{
    uint8_t dataBuffer[6];
    RawImuData sensorData;

    spiDriver_->read(0x0C, dataBuffer, 6);
    int16_t rawGyroX = static_cast<int16_t>((dataBuffer[1] << 8) | dataBuffer[0]);
    int16_t rawGyroY = static_cast<int16_t>((dataBuffer[3] << 8) | dataBuffer[2]);

    sensorData.gyroX = rawGyroX / 16.384f;
    sensorData.gyroY = rawGyroY / 16.384f;

    spiDriver_->read(0x12, dataBuffer, 6);
    int16_t rawAccelX = static_cast<int16_t>((dataBuffer[1] << 8) | dataBuffer[0]);
    int16_t rawAccelY = static_cast<int16_t>((dataBuffer[3] << 8) | dataBuffer[2]);
    int16_t rawAccelZ = static_cast<int16_t>((dataBuffer[5] << 8) | dataBuffer[4]);

    sensorData.accelX = rawAccelX / 16384.0f;
    sensorData.accelY = rawAccelY / 16384.0f;
    sensorData.accelZ = rawAccelZ / 16384.0f;

    return sensorData;
}

#else

inline void GyroscopeReader::initialiseHardware() {}

inline RawImuData GyroscopeReader::readRawImu()
{
    return RawImuData{};
}

#endif

