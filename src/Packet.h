#ifndef PACKET_H
#define PACKET_H

#include <Arduino.h>

struct Packet
{
    uint8_t type;
    uint8_t source;
    uint8_t destination;
    uint8_t sequence;
    uint8_t length;
    uint8_t payload[45];
    uint8_t crc;
};
class PacketUtils
{
  public:
    static void setPayload(Packet &tx, const String &myString); 
    static String getPayload(uint8_t *payload, uint8_t length);
    
    static void setPacket(Packet &tx, const String &myString, uint8_t type, uint8_t source, uint8_t destination, uint8_t sequence);
    static void toRaw(Packet &tx, uint8_t *rawTxPacket);
    static void fromRaw(Packet &rx, uint8_t *rawRxPacket);
};

#endif