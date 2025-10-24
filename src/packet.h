#pragma once
#include "constants.h"

struct Packet
{
    uint8_t type;
    uint8_t source;
    uint8_t destination;
    uint8_t sequence;
    uint8_t length;
    uint8_t payload[200];
};

extern Packet tx;
extern Packet rx;

extern  uint8_t buffer[206];

void setPayload(Packet &tx);
//void setPacket(Packet &tx);
void toRaw(Packet &tx);
void fromRaw(uint8_t buffer[], int len);
String getPayload(Packet packet);

