#include "packet.h"

void setPayload(Packet &packet)
{
    const String &myString = getPayload(packet);
    int len = min((int)myString.length(), (int)sizeof(packet.payload));
    for (int i = 0; i < len; i++)
    {
        packet.payload[i] = myString[i];
    }
}

std::vector<uint8_t> toRaw(Packet &packet)
{
    std::vector<uint8_t> buffer;

    buffer.push_back(packet.type);
    
    buffer.insert(buffer.end(), packet.source, packet.source + 6);
    buffer.insert(buffer.end(), packet.destination, packet.destination + 6);
    buffer.insert(buffer.end(), packet.uuid, packet.uuid + 6);

    buffer.push_back(packet.segmentIndex);
    buffer.push_back(packet.totalSegments);
    buffer.push_back(packet.length);
    
    buffer.insert(buffer.end(), packet.payload, packet.payload + packet.length);

    return buffer;
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
