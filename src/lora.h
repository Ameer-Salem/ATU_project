#pragma once
#include <RadioLib.h>
#include "simulation.h"
#include <queue>
#include "packet.h"

extern std::queue<Packet> outgoingQueue;
extern std::queue<Packet> ingoingQueue;
extern std::queue<Packet> outgoingAckQueue;
extern std::queue<Packet> ingoingAckQueue;

// ===================== LoRa Globals =====================
extern SX1262 lora;
extern int state;
extern volatile bool operationDone;
extern bool transmitFlag;


void loraBegin(float freq, float bw, int sf, int cr );
void startListening();
void receive();
//void sendText(String &str);
void sendPacket(Packet &tx);