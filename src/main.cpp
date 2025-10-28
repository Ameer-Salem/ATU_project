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

    // Handle ACK timeout
    if (waitingForAck && (millis() - ackStartTime > ACK_TIMEOUT))
    {
        retrySend();
    }

    // Handle BLE message
    if (!outgoingQueue.empty() && !waitingForAck)
    {
        Packet nextPacket = outgoingQueue.front();
        outgoingQueue.pop();
        sendPacket(nextPacket);
    }
    if (!ingoingQueue.empty())
    {
        Packet packet = ingoingQueue.front();
        toRaw(packet);
        ingoingQueue.pop();
        txChar->setValue(buffer, sizeof(buffer));
        txChar->notify();
        sLog(BLE_TAG, "Forwarded Buffered TEXT: " + getPayload(packet));
    }
    // Print stats
    if (millis() - lastStatsTime > STATS_INTERVAL)
    {
        lastStatsTime = millis();
        sLog(LORA_TAG, "STATS: sent=" + String(totalSent) + " acks=" + String(totalAcks) + " retries=" + String(totalRetries) + " failed=" + String(totalFailed));
    }
}
