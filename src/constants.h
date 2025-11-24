#pragma once
#include "slog.h"

// ===== BLE_UUIDs =====
inline const char* SERVICE_UUID =               "ffffffff-ffff-ffff-ffff-ffffffffffff";
inline const char* CHARACTERISTIC_UUID_notify = "ffffffff-ffff-ffff-ffff-fffffffffff0";
inline const char* CHARACTERISTIC_UUID_read =   "ffffffff-ffff-ffff-ffff-ffffffffff00";
inline const char* CHARACTERISTIC_UUID_write =  "ffffffff-ffff-ffff-ffff-fffffffff000";

// ===== Log Tags =====
inline const String BLE_TAG = "[BLE]    ";
inline const String LORA_TAG = "[SX1262] ";

// ===== Node Info =====
extern String NODE_ID;
extern uint64_t intNODE_ID;

inline const String NODE_NAME = "Node " + String(NODE_ID);

// ===== Protocol Constants =====
inline constexpr int TEXT_TYPE = 0x01;
inline constexpr int ACK_TYPE = 0x00;
