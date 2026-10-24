#include "Sensors.h"

#include <Arduino.h>
#include <Wire.h>
#include <MAX30105.h>
#include <spo2_algorithm.h>
#include <Adafruit_MLX90614.h>
#include <math.h>

#include "AppState.h"
#include "Config.h"

namespace {
MAX30105 particleSensor;
Adafruit_MLX90614 mlxSensor;

bool wakeMPU6050() {
  Wire.beginTransmission(Config::MPU_ADDRESS);
  if (Wire.endTransmission() != 0) return false;

  Wire.beginTransmission(Config::MPU_ADDRESS);
  Wire.write(0x6B); // PWR_MGMT_1
  Wire.write(0x00); // Wake from sleep
  if (Wire.endTransmission() != 0) return false;
  delay(100);

  Wire.beginTransmission(Config::MPU_ADDRESS);
  Wire.write(0x1C); // ACCEL_CONFIG
  Wire.write(0x00); // +/-2g, 16384 LSB/g
  Wire.endTransmission();
  delay(20);

  Wire.beginTransmission(Config::MPU_ADDRESS);
  Wire.write(0x75); // WHO_AM_I
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(Config::MPU_ADDRESS, static_cast<uint8_t>(1), true) != 1) return false;

  uint8_t whoAmI = Wire.read();
  Serial.print("MPU6050 WHO_AM_I: 0x");
  Serial.println(whoAmI, HEX);
  return whoAmI != 0x00 && whoAmI != 0xFF;
}
} // namespace

void beginSensors() {
  state.maxOk = particleSensor.begin(Wire, I2C_SPEED_STANDARD);
  if (state.maxOk) {
    particleSensor.setup(0x1F, 2, 2, 200, 411, 16384);
    particleSensor.setPulseAmplitudeRed(0x1F);
    particleSensor.setPulseAmplitudeIR(0x1F);
    particleSensor.setPulseAmplitudeGreen(0);
    Serial.println("MAX30102 detected");
  } else {
    Serial.println("MAX30102 not found");
  }

  state.mlxOk = mlxSensor.begin(Config::MLX_ADDRESS, &Wire);
  Serial.println(state.mlxOk ? "MLX90614 detected" : "MLX90614 not found");

  state.mpuOk = wakeMPU6050();
  Serial.println(state.mpuOk ? "MPU6050 detected and awake" : "MPU6050 not working");
}

void readMAX30102() {
  if (!state.maxOk) return;

  state.rawIR = particleSensor.getIR();
  state.rawRed = particleSensor.getRed();

  if (state.rawIR < 20000 || state.rawIR >= 260000 || state.rawRed >= 260000) {
    state.heartRate = state.lastValidHR;
    state.spo2Value = state.lastValidSpO2;
    state.hrValid = state.lastValidHR > 0;
    state.spo2Valid = state.lastValidSpO2 > 0;
    return;
  }

  for (int i = 0; i < Config::PPG_BUFFER_LENGTH; ++i) {
    // A timeout prevents a disconnected/stalled sensor from locking the whole firmware.
    unsigned long start = millis();
    while (!particleSensor.available()) {
      particleSensor.check();
      if (millis() - start > 2500) {
        Serial.println("MAX30102 sample timeout");
        return;
      }
      delay(1);
    }
    state.redBuffer[i] = particleSensor.getRed();
    state.irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(
      state.irBuffer, Config::PPG_BUFFER_LENGTH, state.redBuffer,
      &state.spo2Value, &state.spo2Valid, &state.heartRate, &state.hrValid);

  if (state.hrValid && state.heartRate >= 40 && state.heartRate <= 180)
    state.lastValidHR = state.heartRate;
  else
    state.heartRate = state.lastValidHR;

  if (state.spo2Valid && state.spo2Value >= 70 && state.spo2Value <= 100)
    state.lastValidSpO2 = state.spo2Value;
  else
    state.spo2Value = state.lastValidSpO2;

  state.hrValid = state.lastValidHR > 0;
  state.spo2Valid = state.lastValidSpO2 > 0;
}

void readMLX90614() {
  if (!state.mlxOk) {
    state.tempValid = false;
    return;
  }
  if (millis() - state.lastTempReadTime < Config::TEMP_INTERVAL_MS) return;
  state.lastTempReadTime = millis();

  // MLX90614 may clock-stretch; temporarily lower I2C frequency.
  Wire.setClock(50000);
  delay(5);
  float ambientTemp = mlxSensor.readAmbientTempC();
  delay(5);
  float objectTemp = mlxSensor.readObjectTempC();
  Wire.setClock(100000);

  if (!isnan(objectTemp) && !isnan(ambientTemp) && objectTemp > -20 && objectTemp < 120) {
    state.temperature = objectTemp;
    state.tempValid = true;
    state.mlxFailCount = 0;
    Serial.print("MLX90614 object temp: ");
    Serial.println(state.temperature);
    return;
  }

  state.mlxFailCount++;
  state.tempValid = false;
  state.temperature = 0.0f;
  Serial.println("MLX90614 live read failed: NaN");

  if (state.mlxFailCount >= 3) {
    Serial.println("Reinitializing MLX90614...");
    Wire.setClock(50000);
    state.mlxOk = mlxSensor.begin(Config::MLX_ADDRESS, &Wire);
    Wire.setClock(100000);
    state.mlxFailCount = 0;
  }
}

void readMPU6050() {
  if (!state.mpuOk) {
    state.fallDetected = false;
    return;
  }

  Wire.beginTransmission(Config::MPU_ADDRESS);
  Wire.write(0x3B); // ACCEL_XOUT_H
  if (Wire.endTransmission(false) != 0) {
    state.fallDetected = false;
    Serial.println("MPU6050 read failed");
    return;
  }

  int bytesRead = Wire.requestFrom(Config::MPU_ADDRESS, static_cast<uint8_t>(6), true);
  if (bytesRead != 6) {
    state.fallDetected = false;
    Serial.println("MPU6050 incomplete accel read");
    return;
  }

  int16_t ax = (Wire.read() << 8) | Wire.read();
  int16_t ay = (Wire.read() << 8) | Wire.read();
  int16_t az = (Wire.read() << 8) | Wire.read();

  float x = ax / 16384.0f;
  float y = ay / 16384.0f;
  float z = az / 16384.0f;
  state.totalAccel = sqrtf(x * x + y * y + z * z);
  float accelChange = fabsf(state.totalAccel - state.previousAccel);
  state.previousAccel = state.totalAccel;

  // Retains the original demo's movement heuristic, NOT verified fall detection.
  if (state.totalAccel > 1.6f || state.totalAccel < 0.55f || accelChange > 0.35f) {
    state.fallWarningUntil = millis() + Config::WARNING_HOLD_MS;
    Serial.print("MPU motion detected | Accel: ");
    Serial.print(state.totalAccel);
    Serial.print(" | Change: ");
    Serial.println(accelChange);
  }

  state.fallDetected = millis() < state.fallWarningUntil;
}
