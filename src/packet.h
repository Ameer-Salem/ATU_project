#pragma once
#include "constants.h"

struct Packet
{
    uint8_t type;
    uint8_t source[6];
    uint8_t destination[6];
    uint8_t uuid[6];
    uint8_t segmentIndex;
    uint8_t totalSegments;
    uint8_t length;
    uint8_t payload[200];
};


extern  uint8_t buffer[255];

void setPayload(Packet &packet);
//void setPacket(Packet &tx);
int toRaw(Packet &packet);
Packet fromRaw(uint8_t buffer[], int len);
String getPayload(Packet &packet);

