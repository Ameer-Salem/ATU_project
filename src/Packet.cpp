#include "Packet.h"

void PacketUtils::setPayload(Packet &tx, const String &myString)
{
    for (int i = 0; i < myString.length(); i++)
    {
        tx.payload[i] = myString.charAt(i);
    }
}

String PacketUtils::getPayload(uint8_t *payload, uint8_t length)
{
    String result = "";
    for (int i = 0; i < length; i++)
    {
        result += (char)payload[i];
    }
    return result;
}

void PacketUtils::setPacket(Packet &tx, const String &myString, uint8_t type, uint8_t source, uint8_t destination, uint8_t sequence)
{
    tx.type = type;
    tx.source = source;
    tx.destination = destination;
    tx.sequence = sequence;
    tx.length = myString.length();
    setPayload(tx, myString);
    tx.crc = 0xF3;
}

void PacketUtils::toRaw(Packet &tx, uint8_t *rawTxPacket)
{
    rawTxPacket[0] = tx.type;
    rawTxPacket[1] = tx.source;
    rawTxPacket[2] = tx.destination;
    rawTxPacket[3] = tx.sequence;
    rawTxPacket[4] = tx.length;
    for (int i = 0; i < tx.length; i++)
    {
        rawTxPacket[5 + i] = tx.payload[i];
    }
    rawTxPacket[5 + tx.length] = tx.crc;
}

void PacketUtils::fromRaw(Packet &rx, uint8_t *rawRxPacket)
{
    rx.type = rawRxPacket[0];
    rx.source = rawRxPacket[1];
    rx.destination = rawRxPacket[2];
    rx.sequence = rawRxPacket[3];
    rx.length = rawRxPacket[4];
    for (int i = 0; i < rx.length; i++)
    {
        rx.payload[i] = rawRxPacket[5 + i];
    }
    rx.crc = rawRxPacket[5 + rx.length];
}

