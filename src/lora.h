#pragma once
#include <RadioLib.h>
#include "simulation.h"
#include "ble.h"
#include <queue>

extern std::queue<Packet> outgoingQueue;
extern std::queue<Packet> ingoingQueue;

// ===================== LoRa Globals =====================
extern SX1262 lora;
extern int state;
extern volatile bool operationDone;
extern bool transmitFlag;

// ===================== Sequence & ACK Control =====================
 static uint8_t sequenceCounter;
 static uint8_t lastSequence;
extern unsigned long ackStartTime;
extern int retryCount;
extern bool waitingForAck;

void loraBegin(float freq, float bw, int sf, int cr );
void startListening();
void receive();
//void sendText(String &str);
void retrySend();
void sendAck();
void sendPacket(Packet &tx);