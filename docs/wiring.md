# Wiring reference (ESP32 DevKit)

| Part | ESP32 pin | Purpose |
|---|---|---|
| Shared I2C SDA | GPIO 21 | OLED, MAX30102, MLX90614, MPU6050 |
| Shared I2C SCL | GPIO 22 | OLED, MAX30102, MLX90614, MPU6050 |
| Green LED | GPIO 5 | NORMAL |
| Yellow LED | GPIO 18 | CALIBRATING |
| Red LED | GPIO 19 | WARNING / CRITICAL |
| Buzzer | GPIO 32 | Warning beep / critical triple beep |
| Push button | GPIO 4 to GND | Emergency, `INPUT_PULLUP` |

Common I2C addresses: OLED `0x3C`, MAX30102 `0x57`, MLX90614 `0x5A`, MPU6050 `0x68` (sometimes `0x69`). Check your individual module and use an I2C scanner if a sensor is not detected.

**Electrical safety:** Check each breakout's rated voltage, regulator, and pull-up levels. Do not assume every module is safe with 5 V signals. Always share the grounds. Fit LED resistors and use an appropriate buzzer driver if the buzzer exceeds GPIO current capability.
