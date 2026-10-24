#include "WhatsApp.h"

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "AppState.h"
#include "GeneratedSecrets.h" // Created by scripts/prebuild.py from ignored .env.
#include "Utils.h"

namespace {
void sendWhatsApp(const String &chatId, const String &message) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(Secrets::GREEN_API_URL) + "/waInstance" +
               Secrets::GREEN_ID_INSTANCE + "/sendMessage/" + Secrets::GREEN_API_TOKEN;

  if (!http.begin(url)) {
    Serial.println("GREEN-API HTTP begin failed");
    return;
  }
  http.addHeader("Content-Type", "application/json");
  String body = "{\"chatId\":\"" + jsonEscape(chatId) +
                "\",\"message\":\"" + jsonEscape(message) + "\"}";
  int httpCode = http.POST(body);

  Serial.print("GREEN-API HTTP code: ");
  Serial.println(httpCode);
  Serial.println(http.getString());
  http.end();
}
} // namespace

void checkWhatsAppAlert() {
  if (state.riskLevel != "WARNING" && state.riskLevel != "CRITICAL") {
    state.warningAlertSent = false;
    return;
  }
  if (state.warningAlertSent) return;

  String message = state.riskLevel + " ALERT - Bed 1\n";
  message += "Patient: John Doe\n";
  message += "HR: " + String(state.lastValidHR) + " bpm\n";
  message += "SpO2: " + String(state.lastValidSpO2) + "%\n";
  message += "Temp: ";
  message += state.tempValid ? String(state.temperature, 1) : String("---");
  message += " C\n";
  message += "Score: " + String(state.healthScore) + "/100\n";
  message += "Status: " + state.riskLevel + "\n";
  if (state.baselineReady) {
    message += "Baseline HR: " + String(state.baselineHR, 1) + " bpm\n";
    message += "Baseline SpO2: " + String(state.baselineSpO2, 1) + "%\n";
    message += "Baseline Temp: " + String(state.baselineTemp, 1) + " C\n";
  }
  if (state.fallDetected) message += "FALL DETECTED\n";
  if (state.manualEmergency) message += "EMERGENCY BUTTON PRESSED\n";
  message += "Red LED is ON\n";
  message += "Time: " + getTimeText();

  String familyMessage = "Patient " + state.riskLevel + " Alert\n";
  familyMessage += "Your family member needs attention.\n";
  if (state.fallDetected) familyMessage += "Fall/motion detected.\n";
  if (state.manualEmergency) familyMessage += "Emergency button was pressed.\n";
  familyMessage += "Please contact the hospital.\n";
  familyMessage += "Time: " + getTimeText();

  sendWhatsApp(Secrets::DOCTOR_CHAT_ID, message);
  delay(500);
  sendWhatsApp(Secrets::FAMILY_CHAT_ID, familyMessage);
  state.warningAlertSent = true; // Original: one send attempt per continuous alert state.
}
