#include "Display.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include "AppState.h"
#include "Config.h"

namespace {
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);
}

void beginDisplay() {
  display.setI2CAddress(Config::OLED_ADDRESS * 2);
  display.begin();
  display.setPowerSave(0);
  display.setContrast(255);
  display.clearBuffer();
  display.setFont(u8g2_font_6x13_tf);
  display.drawStr(0, 18, "Patient Monitor");
  display.drawStr(0, 38, "Starting...");
  display.sendBuffer();
}

void updateOLED() {
  display.clearBuffer();
  display.setFont(u8g2_font_6x13_tf);
  display.drawStr(0, 10, "PATIENT MONITOR");

  if (!state.baselineReady) {
    unsigned long elapsed = state.baselineStarted ? millis() - state.baselineStartTime : 0;
    int remaining = (Config::BASELINE_DURATION_MS - min(elapsed, Config::BASELINE_DURATION_MS)) / 1000;
    display.setCursor(0, 24);
    display.print("Calibrating...");
    display.setCursor(0, 38);
    display.print("Keep finger placed");
    display.setCursor(0, 52);
    display.print("Time left: ");
    display.print(remaining);
    display.print("s");
    display.sendBuffer();
    return;
  }

  display.setCursor(0, 22);
  display.print("HR: ");
  if (state.hrValid) display.print(state.heartRate);
  else display.print("---");
  display.print(" bpm");

  display.setCursor(0, 34);
  display.print("SpO2: ");
  if (state.spo2Valid) display.print(state.spo2Value);
  else display.print("---");
  display.print(" %");

  display.setCursor(0, 46);
  display.print("Temp: ");
  if (state.tempValid) display.print(state.temperature, 1);
  else display.print("---");
  display.print(" C");

  display.setCursor(0, 58);
  display.print("Status: ");
  display.print(state.riskLevel);
  if (state.fallDetected) {
    display.setCursor(92, 58);
    display.print("FALL");
  }
  display.sendBuffer();
}
