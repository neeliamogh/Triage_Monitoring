#pragma once

#include <Arduino.h>
#include "Config.h"

// A single shared state object avoids dozens of scattered global variables.
struct MonitorState {
  bool maxOk = false;
  bool mlxOk = false;
  bool mpuOk = false;

  uint32_t irBuffer[Config::PPG_BUFFER_LENGTH] = {};
  uint32_t redBuffer[Config::PPG_BUFFER_LENGTH] = {};
  int32_t spo2Value = 0;
  int8_t spo2Valid = 0;
  int32_t heartRate = 0;
  int8_t hrValid = 0;
  int lastValidHR = 0;
  int lastValidSpO2 = 0;
  long rawIR = 0;
  long rawRed = 0;

  float temperature = 0.0f;
  bool tempValid = false;
  float totalAccel = 1.0f;
  float previousAccel = 1.0f;

  int healthScore = 100;
  String riskLevel = "NORMAL";

  bool baselineReady = false;
  bool baselineStarted = false;
  unsigned long baselineStartTime = 0;
  unsigned long lastBaselineSampleTime = 0;
  float baselineHR = 0.0f;
  float baselineSpO2 = 0.0f;
  float baselineTemp = 0.0f;
  float baselineHRSum = 0.0f;
  float baselineSpO2Sum = 0.0f;
  float baselineTempSum = 0.0f;
  int baselineHRCount = 0;
  int baselineSpO2Count = 0;
  int baselineTempCount = 0;

  bool fallDetected = false;
  bool manualEmergency = false;
  bool warningAlertSent = false;
  bool lastButtonState = HIGH;
  unsigned long lastButtonTime = 0;
  unsigned long lastTempReadTime = 0;
  unsigned long buttonWarningUntil = 0;
  unsigned long fallWarningUntil = 0;
  byte mlxFailCount = 0;

  int hrHistory[Config::HISTORY_LENGTH] = {};
  int spo2History[Config::HISTORY_LENGTH] = {};
  float tempHistory[Config::HISTORY_LENGTH] = {};
  int historyIndex = 0;
};

extern MonitorState state;
