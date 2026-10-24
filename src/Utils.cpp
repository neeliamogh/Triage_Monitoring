#include "Utils.h"
#include <time.h>

String getTimeText() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    unsigned long seconds = millis() / 1000;
    char text[12];
    snprintf(text, sizeof(text), "%02d:%02d:%02d",
             int((seconds / 3600) % 24), int((seconds / 60) % 60),
             int(seconds % 60));
    return String(text);
  }
  char text[12];
  strftime(text, sizeof(text), "%H:%M:%S", &timeinfo);
  return String(text);
}

String jsonEscape(String text) {
  text.replace("\\", "\\\\");
  text.replace("\"", "\\\"");
  text.replace("\n", "\\n");
  return text;
}
