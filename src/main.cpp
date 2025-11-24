// ===================== Includes =====================
#include "ble.h"

// ===================== LoRa Control =====================
void setFlag(void)
{
    operationDone = true;
}
bool canTransmit()
{
    return !transmitFlag && !operationDone;
}

// ===================== Arduino Setup =====================
void setup()
{
    randomSeed(analogRead(13));
    Serial.begin(115200);

    // BLE setup
    bleSetup();

    // LoRa setup
    loraBegin(434.0, 125.0, 9, 6);
    lora.setDio1Action(setFlag);
    startListening();
}

// ===================== Arduino Loop =====================
void loop()
{
    // Handle BLE ACK
    if (!ingoingAckQueue.empty())
    {
        Packet ack = ingoingAckQueue.front();
        
        std::vector<uint8_t> buffer = toRaw(ack);
        ingoingAckQueue.pop();
        notifyBLE(buffer);
    }
    
    // Handle LoRa ACK
    if (!outgoingAckQueue.empty() && canTransmit())
    {
        Packet ack = outgoingAckQueue.front();
        
        outgoingAckQueue.pop();
        sendPacket(ack);
    }
    // Handle LoRa message
    if (!outgoingQueue.empty() && !transmitFlag && outgoingAckQueue.empty())
    {
        Packet tx = outgoingQueue.front();
        outgoingQueue.pop();
        sendPacket(tx);
    }
    // Handle BLE message
    if (!ingoingQueue.empty() && ingoingAckQueue.empty())
    {
        Packet packet = ingoingQueue.front();
        std::vector<uint8_t> buffer = toRaw(packet);
        ingoingQueue.pop();
        notifyBLE(buffer);
        sLog(BLE_TAG, "Forwarded Buffered TEXT: " + getPayload(packet));
    }
    
    
    
    // Handle LoRa events
    if (operationDone)
    {
        sLog(LORA_TAG, "Previous operation finished");
        operationDone = false;
        if (transmitFlag)
        {
            sLog(LORA_TAG, "Previous operation was transmission");
            transmitFlag = false;
            startListening();
            return;
        }
        else
        {
            sLog(LORA_TAG, "Previous operation was reception");
            receive();
        }
    }
}
