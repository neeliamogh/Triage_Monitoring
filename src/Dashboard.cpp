#include "Dashboard.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "AppState.h"
#include "GeneratedAssets.h" // Auto-generated from web/ by scripts/prebuild.py.
#include "Utils.h"

namespace {
WebServer server(80);

String historyJsonInt(const int *values) {
  String json = "[";
  for (int i = 0; i < Config::HISTORY_LENGTH; ++i) {
    if (i) json += ",";
    json += String(values[i]);
  }
  return json + "]";
}

String historyJsonFloat(const float *values) {
  String json = "[";
  for (int i = 0; i < Config::HISTORY_LENGTH; ++i) {
    if (i) json += ",";
    json += String(values[i], 1);
  }
  return json + "]";
}

String patientJson(const String &name, int hr, int sp, float temp,
                   int score, const String &status, bool fall) {
  String json = "{";
  json += "\"name\":\"" + jsonEscape(name) + "\",";
  json += "\"hr\":" + String(hr) + ",";
  json += "\"spo2\":" + String(sp) + ",";
  json += "\"temp\":" + String(temp, 1) + ",";
  json += "\"score\":" + String(score) + ",";
  json += "\"status\":\"" + jsonEscape(status) + "\",";
  json += "\"fall\":" + String(fall ? "true" : "false") + ",";
  json += "\"updated\":\"" + getTimeText() + "\"";
  return json + "}";
}

void handleData() {
  // Beds 2 and 3 are FAKE DATA, as in the original project sketch.
  int bed2HR = 78 + random(-4, 5);
  int bed2SpO2 = 97 + random(-1, 2);
  float bed2Temp = 36.6f + random(-2, 3) * 0.1f;
  int bed2Score = 88 + random(-3, 4);

  int bed3HR = 82 + random(-4, 5);
  int bed3SpO2 = 97 + random(-1, 2);
  float bed3Temp = 36.7f + random(-2, 3) * 0.1f;
  int bed3Score = 88 + random(-3, 4);

  String json = "{";
  json += "\"bed1\":" + patientJson("John Doe", state.lastValidHR,
      state.lastValidSpO2, state.tempValid ? state.temperature : 0,
      state.healthScore, state.riskLevel, state.fallDetected) + ",";
  json += "\"bed2\":" + patientJson("Priya Sharma", bed2HR,
      bed2SpO2, bed2Temp, bed2Score, "NORMAL", false) + ",";
  json += "\"bed3\":" + patientJson("Raju Verma", bed3HR,
      bed3SpO2, bed3Temp, bed3Score, "NORMAL", false) + ",";
  json += "\"timestamp\":\"" + getTimeText() + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"wifi\":\"" + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected") + "\",";
  json += "\"baseline\":" + String(state.baselineReady ? "true" : "false") + ",";
  json += "\"hrHistory\":" + historyJsonInt(state.hrHistory) + ",";
  json += "\"spo2History\":" + historyJsonInt(state.spo2History) + ",";
  json += "\"tempHistory\":" + historyJsonFloat(state.tempHistory);
  server.send(200, "application/json", json + "}");
}
} // namespace

void beginDashboard() {
  server.on("/", HTTP_GET, []() { server.send_P(200, "text/html", Assets::INDEX_HTML); });
  server.on("/styles.css", HTTP_GET, []() { server.send_P(200, "text/css", Assets::STYLES_CSS); });
  server.on("/app.js", HTTP_GET, []() { server.send_P(200, "application/javascript", Assets::APP_JS); });
  server.on("/data", HTTP_GET, handleData);
  server.begin();
}

void serviceDashboard() {
  server.handleClient();
}
