#pragma once
#include "Arduino.h"
void sLog(const String tag, const String &content);
void logBytes(const String tag, const String name, uint8_t *data, int size);
