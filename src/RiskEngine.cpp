#include "RiskEngine.h"
#include "AppState.h"

#include <math.h>

void computeHealthScore() {
  if (!state.baselineReady) {
    state.healthScore = 100;
    state.riskLevel = "CALIBRATING";
  } else {
    int criticalPoints = 0;

    if (state.hrValid && state.lastValidHR > 0) {
      float hrDiff = fabsf(state.lastValidHR - state.baselineHR);
      if (hrDiff >= 45) criticalPoints += 2;
    }
    if (state.spo2Valid && state.lastValidSpO2 > 0) {
      float spo2Drop = state.baselineSpO2 - state.lastValidSpO2;
      if (spo2Drop >= 10) criticalPoints += 3;
    }
    if (state.tempValid) {
      float tempDiff = fabsf(state.temperature - state.baselineTemp);
      if (tempDiff >= 3.0f) criticalPoints += 2;
    }

    state.healthScore = constrain(100 - criticalPoints * 25, 0, 100);

    // Original prototype behavior: only large changes can trigger CRITICAL.
    // WARNING is reserved for button/motion events and is shown for 5 seconds.
    if (criticalPoints >= 4 || state.healthScore <= 25)
      state.riskLevel = "CRITICAL";
    else
      state.riskLevel = "NORMAL";
  }

  state.manualEmergency = millis() < state.buttonWarningUntil;
  state.fallDetected = millis() < state.fallWarningUntil;
  if (state.fallDetected || state.manualEmergency) state.riskLevel = "WARNING";
}
