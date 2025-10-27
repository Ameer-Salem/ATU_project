#include "packet.h"

Packet tx;
Packet rx;
uint8_t buffer[206];
void setPayload(Packet &tx )
{
    const String &myString = getPayload(tx);
    int len = min((int)myString.length(), (int)sizeof(tx.payload));
    for (int i = 0; i < len; i++)
    {
        tx.payload[i] = myString[i];
    }
}

// void setPacket(Packet &tx, const String &myString, uint8_t type, uint8_t source, uint8_t destination, uint8_t sequence)
// {
//     tx.type = type;
//     tx.source = source;
//     tx.destination = destination;
//     tx.sequence = sequence;
//     tx.length = myString.length();
//     setPayload(tx);
// }

void toRaw(Packet &tx)
{
    buffer[0] = tx.type;
    buffer[1] = tx.source;
    buffer[2] = tx.destination;
    buffer[3] = tx.sequence;
    buffer[4] = tx.length;
    
    for (int i = 0; i < tx.length; i++)
    {
        buffer[5 + i] = tx.payload[i];
    }
}

void fromRaw(uint8_t buffer[], int len)
{
    rx.type = buffer[0];
    rx.source = buffer[1];
    rx.destination = buffer[2];
    rx.sequence = buffer[3];
    rx.length = buffer[4];
    int safeLen = min(rx.length, (uint8_t)sizeof(rx.payload));
    for (int i = 0; i < safeLen; i++)
    {
        rx.payload[i] = buffer[5 + i];
    }
    rx.length = safeLen;
}

String getPayload(Packet packet)
{
    return String((char *)packet.payload, packet.length);
}