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
void logBytes(const String tag, const String name, uint8_t *data, int size)
{
    unsigned long now = millis();
    char timeStr[20];
    snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu:%02lu", now / 3600000, (now / 60000) % 60, (now / 1000) % 60);
    Serial.print(timeStr);
    Serial.print(" ");
    Serial.print(tag.c_str());
    Serial.print(" ");
    Serial.print(name.c_str());
    Serial.print(": ");
    for (int i = 0; i < size; i++)
    {
        if (data[i] < 16)
            Serial.print("0"); // for leading zero
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}