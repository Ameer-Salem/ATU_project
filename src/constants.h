#pragma once
#include "slog.h"

// ===== BLE_UUIDs =====
inline const char* SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef0";
inline const char* CHARACTERISTIC_UUID_RX = "12345678-1234-5678-1234-56789abcdef1";
inline const char* CHARACTERISTIC_UUID_TX = "12345678-1234-5678-1234-56789abcdef2";

// ===== Log Tags =====
inline const String BLE_TAG = "[BLE]    ";
inline const String LORA_TAG = "[SX1262] ";

// ===== Node Info =====
inline constexpr int NODE_ID = 1;
inline const String NODE_NAME = "Node " + String(NODE_ID);

// ===== Protocol Constants =====
inline constexpr int MAX_RETRIES = 3;
inline constexpr int ACK_TIMEOUT = 5000;
inline constexpr int SOURCE_ID = NODE_ID;
inline constexpr int DESTINATION_ID = 2;
inline constexpr int TEXT_TYPE = 0x01;
inline constexpr int ACK_TYPE = 0x02;
