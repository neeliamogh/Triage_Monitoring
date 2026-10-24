#pragma once

#include <Arduino.h>

namespace Config {
constexpr uint8_t I2C_SDA = 21;
constexpr uint8_t I2C_SCL = 22;

constexpr uint8_t GREEN_LED = 5;
constexpr uint8_t YELLOW_LED = 18;
constexpr uint8_t RED_LED = 19;
constexpr uint8_t BUZZER = 32;
constexpr uint8_t EMERGENCY_BUTTON = 4;

constexpr uint8_t MPU_ADDRESS = 0x68;  // Change to 0x69 if your scan shows 0x69.
constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr uint8_t MLX_ADDRESS = 0x5A;

constexpr int PPG_BUFFER_LENGTH = 50;
constexpr int HISTORY_LENGTH = 10;
constexpr unsigned long BASELINE_DURATION_MS = 60000;
constexpr unsigned long WARNING_HOLD_MS = 5000;
constexpr unsigned long TEMP_INTERVAL_MS = 1200;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 350;
} // namespace Config
