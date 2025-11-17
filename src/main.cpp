// ===================== Includes =====================
#include "ble.h"

// ===================== LoRa Control =====================
void setFlag(void)
{
    operationDone = true;
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
        }
        else
        {
            sLog(LORA_TAG, "Previous operation was reception");
            receive();
        }
    }

    if (!ackQueue.empty() && !transmitFlag && !operationDone)
    {
        Packet ack = ackQueue.front();

        if ((uint64_t)ack.destination == intNODE_ID)
        {
            int len = toRaw(ack);
            ackQueue.pop();
            notifyChar->setValue(buffer, len);
            notifyChar->notify();
            memset(buffer, 0, sizeof(buffer));
        }
        else
        {
            sendPacket(ack);
            ackQueue.pop();
        }
    }
    // Handle BLE message
    if (!outgoingQueue.empty() && !transmitFlag && ackQueue.empty())
    {
        Packet tx = outgoingQueue.front();
        outgoingQueue.pop();
        sendPacket(tx);
    }
    if (!ingoingQueue.empty() && ackQueue.empty())
    {
        Packet packet = ingoingQueue.front();
        int len = toRaw(packet);
        ingoingQueue.pop();
        notifyChar->setValue(buffer, len);
        notifyChar->notify();
        memset(buffer, 0, sizeof(buffer));
        sLog(BLE_TAG, "Forwarded Buffered TEXT: " + getPayload(packet));
    }
}
