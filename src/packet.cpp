#include "packet.h"

uint8_t buffer[255];
void setPayload(Packet &packet)
{
    const String &myString = getPayload(packet);
    int len = min((int)myString.length(), (int)sizeof(packet.payload));
    for (int i = 0; i < len; i++)
    {
        packet.payload[i] = myString[i];
    }
}

int toRaw(Packet &packet)
{
    buffer[0] = packet.type;
    for (int i = 0; i < sizeof(packet.source); i++)
    {
        buffer[1 + i] = packet.source[i];
    }
    for (int i = 0; i < sizeof(packet.destination); i++)
    {
        buffer[7 + i] = packet.destination[i];
    }
    for (int i = 0; i < sizeof(packet.uuid); i++)
    {
        buffer[13 + i] = packet.uuid[i];
    }
    buffer[19] = packet.segmentIndex;
    buffer[20] = packet.totalSegments;
    buffer[21] = packet.length;
    
    for (int i = 0; i < packet.length; i++)
    {
        buffer[22 + i] = packet.payload[i];
    }
    return 22 + packet.length;
}

Packet fromRaw(uint8_t buffer[], int len)
{
    Packet packet;
    packet.type = buffer[0];
    for (int i = 0; i < 6; i++)
    {
        packet.source[i] = buffer[1 + i];
    }
    for (int i = 0; i < sizeof(packet.destination); i++)
    {
        packet.destination[i] = buffer[7 + i];
    }
    for (int i = 0; i < sizeof(packet.uuid); i++)
    {
        packet.uuid[i] = buffer[13 + i];
    }
    packet.segmentIndex = buffer[19];
    packet.totalSegments = buffer[20];
    packet.length = buffer[21];
    int safeLen = min(packet.length, (uint8_t)sizeof(packet.payload));
    for (int i = 0; i < safeLen; i++)
    {
        packet.payload[i] = buffer[22 + i];
    }
    packet.length = safeLen;
    return packet;
}

String getPayload(Packet &packet)
{
    return String((char *)packet.payload, packet.length);
}
