#ifndef TEXT_TYPE
#define TEXT_TYPE 0x01
#endif
#ifndef ACK_TYPE
#define ACK_TYPE 0x02
#endif
#ifndef LoRa_HANDLER_H
#define LoRa_HANDLER_H


#include <RadioLib.h>
#include "Packet.h"

class LoRaHandler
{
public:

    LoRaHandler(int SS, int DIO0, int RST_LoRa, int BUSY_LoRa);

    bool begin();
    void sendText(const String &msg, uint8_t src, uint8_t des, uint8_t seq);
    void sendAck(uint8_t seq, uint8_t des);
    bool receive(Packet &rx);

private:
    SX1262 lora;
    int state;
    uint8_t rxPacket[60];
    uint8_t txPacket[60];
};

#endif