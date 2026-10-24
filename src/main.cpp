#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <time.h>

#include "Alerts.h"
#include "AppState.h"
#include "Baseline.h"
#include "Config.h"
#include "Dashboard.h"
#include "Display.h"
#include "GeneratedSecrets.h"
#include "RiskEngine.h"
#include "Sensors.h"
#include "WhatsApp.h"

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(Config::I2C_SDA, Config::I2C_SCL);
  Wire.setClock(100000);
  delay(300);

  beginDisplay();
  beginSensors();

  WiFi.begin(Secrets::WIFI_SSID, Secrets::WIFI_PASS);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  configTime(19800, 0, "pool.ntp.org", "time.nist.gov"); // UTC+05:30
  beginDashboard();
  randomSeed(analogRead(34));
  beginAlerts();
}

void loop() {
  serviceDashboard();
  handleButton();

  readMAX30102();
  readMLX90614();
  readMPU6050();

  state.heartRate = state.lastValidHR;
  state.spo2Value = state.lastValidSpO2;

  updateBaseline();
  addHistory();
  computeHealthScore();
  updateOLED();
  triggerLocalAlerts();
  checkWhatsAppAlert();

  Serial.print("HR: "); Serial.print(state.lastValidHR);
  Serial.print(" | SpO2: "); Serial.print(state.lastValidSpO2);
  Serial.print(" | Temp: ");
  if (state.tempValid) Serial.print(state.temperature);
  else Serial.print("---");
  Serial.print(" | Score: "); Serial.print(state.healthScore);
  Serial.print(" | Status: "); Serial.print(state.riskLevel);
  Serial.print(" | Accel: "); Serial.print(state.totalAccel);
  Serial.print(" | MAX: "); Serial.print(state.maxOk ? "OK" : "NO");
  Serial.print(" | MLX: "); Serial.print(state.mlxOk ? "OK" : "NO");
  Serial.print(" | MPU: "); Serial.print(state.mpuOk ? "OK" : "NO");
  Serial.print(" | IP: "); Serial.println(WiFi.localIP());

  delay(200);
}
