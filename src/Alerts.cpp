#include "Alerts.h"
#include <Arduino.h>
#include "AppState.h"
#include "Config.h"

namespace {
void buzzerOn() { digitalWrite(Config::BUZZER, HIGH); }
void buzzerOff() { digitalWrite(Config::BUZZER, LOW); }
void beepOnce(int durationMs) {
  buzzerOn();
  delay(durationMs);
  buzzerOff();
}
}

void beginAlerts() {
  pinMode(Config::GREEN_LED, OUTPUT);
  pinMode(Config::YELLOW_LED, OUTPUT);
  pinMode(Config::RED_LED, OUTPUT);
  pinMode(Config::BUZZER, OUTPUT);
  pinMode(Config::EMERGENCY_BUTTON, INPUT_PULLUP);
  buzzerOff();

  // Original startup LED/buzzer self-test.
  digitalWrite(Config::GREEN_LED, HIGH);
  digitalWrite(Config::YELLOW_LED, HIGH);
  digitalWrite(Config::RED_LED, HIGH);
  beepOnce(150);
  delay(200);
  digitalWrite(Config::GREEN_LED, LOW);
  digitalWrite(Config::YELLOW_LED, LOW);
  digitalWrite(Config::RED_LED, LOW);
}

void handleButton() {
  bool buttonState = digitalRead(Config::EMERGENCY_BUTTON);
  if (state.lastButtonState == HIGH && buttonState == LOW &&
      millis() - state.lastButtonTime > Config::BUTTON_DEBOUNCE_MS) {
    state.lastButtonTime = millis();
    state.manualEmergency = true;
    state.buttonWarningUntil = millis() + Config::WARNING_HOLD_MS;
    Serial.println("Button: temporary warning triggered");
  }
  state.lastButtonState = buttonState;
}

void triggerLocalAlerts() {
  digitalWrite(Config::GREEN_LED, LOW);
  digitalWrite(Config::YELLOW_LED, LOW);
  digitalWrite(Config::RED_LED, LOW);
  buzzerOff();

  if (state.riskLevel == "CALIBRATING") {
    digitalWrite(Config::YELLOW_LED, HIGH);
  } else if (state.riskLevel == "NORMAL") {
    digitalWrite(Config::GREEN_LED, HIGH);
  } else if (state.riskLevel == "WARNING") {
    digitalWrite(Config::RED_LED, HIGH); // Warning intentionally uses red in the prototype.
    beepOnce(80);
  } else {
    digitalWrite(Config::RED_LED, HIGH);
    for (int i = 0; i < 3; ++i) {
      beepOnce(90);
      delay(80);
    }
  }
}
