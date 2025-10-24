#include "slog.h"

void sLog(const String tag, const String &content)
{
unsigned long now = millis();
    char timeStr[20];
    snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu:%02lu", now / 3600000, (now / 60000) % 60, (now / 1000) % 60);
    Serial.print(timeStr);
    Serial.print(" ");
    Serial.print(tag);
    Serial.print(" ");
    Serial.println(content);
}