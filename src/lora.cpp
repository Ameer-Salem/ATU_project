#include "lora.h"
#include "packet.h"
#include "ble.h"
SX1262 lora = new Module(SS, DIO0, RST_LoRa, BUSY_LoRa);
int state;

volatile bool operationDone = false;
bool transmitFlag = false;

std::queue<Packet> outgoingQueue;
std::queue<Packet> ingoingQueue;
std::queue<Packet> ackQueue;

void loraBegin(float freq, float bw, int sf, int cr)
{
    state = lora.begin(freq, bw, sf, cr);
    if (state == RADIOLIB_ERR_NONE)
    {
        sLog(LORA_TAG, "LoRa initialized successfully!");
    }
    else
    {
        sLog(LORA_TAG, "LoRa initialization failed, code: " + String(state));
        while (true)
            ;
    }
}

void sendPacket(Packet &tx)
{
    memset(buffer, 0, sizeof(buffer));
    int len = toRaw(tx);

    state = lora.startTransmit(buffer, len);
    if (state == RADIOLIB_ERR_NONE)
    {
        transmitFlag = true;
        sLog(LORA_TAG, "Transmitting ... ");
    }
    else
    {
        sLog(LORA_TAG, "failed, code " + String(state));
    }
}

void startListening()
{
    sLog(LORA_TAG, "Starting to listen ... ");
    int state = lora.startReceive();
    if (state != RADIOLIB_ERR_NONE)
    {
        sLog(LORA_TAG, "failed, code " + String(state));
    }
}

void receive()
{
    memset(buffer, 0, sizeof(buffer));
    int len = lora.getPacketLength();
    int state = lora.readData(buffer, len);
    Packet packet;
    
    if (len < 21 || buffer[21] > sizeof(packet.payload))
    {
        sLog(LORA_TAG, "Invalid packet received");
        return;
    }
    if (state != RADIOLIB_ERR_NONE)
    {
        sLog(LORA_TAG, "failed, code " + String(state));
        return;
    }

    packet = fromRaw(buffer, len);

    if (packet.type == ACK_TYPE)
    {
        sLog(LORA_TAG, "Received ACK! for uid : " + String((char *)packet.uuid) + " - segmentIndex " + String(packet.segmentIndex));
        ackQueue.push(packet);
    }
    else
    {
        ingoingQueue.push(packet);
        sLog(LORA_TAG, "Buffered new TEXT: " + String((char *)packet.uuid) + " - segmentIndex " + String(packet.segmentIndex));
    }
}