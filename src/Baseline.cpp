#include "Baseline.h"
#include "AppState.h"
#include "Config.h"

void addHistory() {
  state.hrHistory[state.historyIndex] = state.lastValidHR;
  state.spo2History[state.historyIndex] = state.lastValidSpO2;
  state.tempHistory[state.historyIndex] = state.tempValid ? state.temperature : 0;
  state.historyIndex = (state.historyIndex + 1) % Config::HISTORY_LENGTH;
}

void updateBaseline() {
  if (state.baselineReady) return;

  if (!state.baselineStarted) {
    state.baselineStarted = true;
    state.baselineStartTime = millis();
    state.lastBaselineSampleTime = 0;
    Serial.println("Baseline calibration started. Keep finger on MAX30102 for 60 seconds.");
  }

  if (millis() - state.lastBaselineSampleTime < 1000) return;
  state.lastBaselineSampleTime = millis();

  if (state.hrValid && state.lastValidHR >= 40 && state.lastValidHR <= 180) {
    state.baselineHRSum += state.lastValidHR;
    state.baselineHRCount++;
  }
  if (state.spo2Valid && state.lastValidSpO2 >= 70 && state.lastValidSpO2 <= 100) {
    state.baselineSpO2Sum += state.lastValidSpO2;
    state.baselineSpO2Count++;
  }
  if (state.tempValid && state.temperature > 20 && state.temperature < 45) {
    state.baselineTempSum += state.temperature;
    state.baselineTempCount++;
  }

  if (millis() - state.baselineStartTime >= Config::BASELINE_DURATION_MS) {
    // Retained from prototype: fallback values are for DEMO only, not clinical baselines.
    state.baselineHR = state.baselineHRCount ? state.baselineHRSum / state.baselineHRCount : 75;
    state.baselineSpO2 = state.baselineSpO2Count ? state.baselineSpO2Sum / state.baselineSpO2Count : 98;
    state.baselineTemp = state.baselineTempCount ? state.baselineTempSum / state.baselineTempCount : 36.5f;
    state.baselineReady = true;

    Serial.println("Baseline calibration complete");
    Serial.print("Baseline HR: "); Serial.print(state.baselineHR);
    Serial.print(" | Baseline SpO2: "); Serial.print(state.baselineSpO2);
    Serial.print(" | Baseline Temp: "); Serial.println(state.baselineTemp);
  }
}
