#include "LoRaHandler.h"

LoRaHandler::LoRaHandler(int SS, int DIO0, int RST_LoRa, int BUSY_LoRa) : lora(new Module(SS, DIO0, RST_LoRa, BUSY_LoRa)){}

bool LoRaHandler::begin(){
    state = lora.begin();
    if (state != RADIOLIB_ERR_NONE) return false;
    
    lora .setOutputPower(20);     // 2-20, higher = more power
    lora.setFrequency(433.0);    // 433.0–915.0, must match frequency of receiver
    lora.setSpreadingFactor(12); // 7–12, higher = more robust, slower
    lora.setCodingRate(6);       // 5–8, higher = more robust, slower
    lora.setBandwidth(250.0);    // kHz, 125 is standard
    lora.setSyncWord(0x12);      // default is 0x12
    lora.setCRC(false);          // false = faster
    return true;
}

void LoRaHandler::sendText(const String &msg, uint8_t src, uint8_t des, uint8_t seq){
    Packet tx;
    PacketUtils::setPacket(tx, msg, TEXT_TYPE ,src, des, seq);
    lora.transmit((uint8_t *)&tx, 6+tx.length+1);

    if (state == RADIOLIB_ERR_NONE)
    {
        /* code */
        Serial.println("Packet sent!");
    }
    else
    {
        /* code */
        Serial.print("Transmit failed, code: ");
        Serial.println(state);
    }
}

void LoRaHandler::sendAck(uint8_t seq, uint8_t des){
    Packet ack;
    ack.type = ACK_TYPE;
    ack.source = 1;
    ack.destination = des;
    ack.sequence = seq;
    ack.length = 0;
    ack.crc = 0xF3;

    PacketUtils::toRaw(ack, txPacket);
    lora.transmit(txPacket, 6);
}

bool LoRaHandler::receive(Packet &rx) {
    state = lora.receive(rxPacket, 60);
    if (state == RADIOLIB_ERR_NONE)
    {
        PacketUtils::fromRaw(rx, rxPacket);
        return true;
    }
    return false;
}
