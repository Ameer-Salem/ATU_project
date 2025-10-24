#include "lora.h"
#include "packet.h"

SX1262 lora = new Module(SS, DIO0, RST_LoRa, BUSY_LoRa);
int state;

int retryCount = 0;
unsigned long ackStartTime = 0;
bool waitingForAck = false;
volatile bool operationDone;
bool transmitFlag;

std::queue<Packet> outgoingQueue;
std::queue<Packet> ingoingQueue;

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
    //setPayload(tx);
    //setPacket(tx);
    toRaw(tx);

    if (SIMULATE_PACKET_LOSS)
    {
        if (tx.type == TEXT_TYPE && DROP_EVERY_NTH > 0 && (tx.sequence % DROP_EVERY_NTH) == 0)
        {
            sLog(LORA_TAG, "SIMULATED DROP of DATA packet seq " + String(tx.sequence));
            state = RADIOLIB_ERR_NONE;
            transmitFlag = true;
            totalSent++;
            return;
        }
        if (tx.type == ACK_TYPE && DROP_ACK_EVERY_NTH > 0 && (tx.sequence % DROP_ACK_EVERY_NTH) == 0)
        {
            sLog(LORA_TAG, "SIMULATED DROP of ACK seq " + String(tx.sequence));
            state = RADIOLIB_ERR_NONE;
            transmitFlag = true;
            return;
        }
        int r = random(100);
        if (tx.type == TEXT_TYPE && r < LOSS_PROBABILITY)
        {
            sLog(LORA_TAG, "SIMULATED RANDOM DROP of DATA packet seq " + String(tx.sequence) + " (r=" + String(r) + ")");
            state = RADIOLIB_ERR_NONE;
            transmitFlag = true;
            totalSent++;
            return;
        }
        if (tx.type == ACK_TYPE && r < LOSS_ACK_PROBABILITY)
        {
            sLog(LORA_TAG, "SIMULATED RANDOM DROP of ACK seq " + String(tx.sequence) + " (r=" + String(r) + ")");
            state = RADIOLIB_ERR_NONE;
            transmitFlag = true;
            return;
        }
    }
    // Real transmit
    state = lora.startTransmit(buffer, tx.length + 5);
    if (state == RADIOLIB_ERR_NONE)
    {
        transmitFlag = true;
        sequenceCounter++;
        totalSent++;
        sLog(LORA_TAG, "transmission started! type : " + String(tx.type == (uint8_t)TEXT_TYPE ? "TEXT" : "ACK") + " seq " + String(tx.sequence));
    }
    else
    {
        sLog(LORA_TAG, "failed, code " + String(state));
    }
}

void sendAck()
{
    Packet packet = {ACK_TYPE, SOURCE_ID, DESTINATION_ID, sequenceCounter, 0};
    sendPacket(packet);
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
void retrySend(){
    if (retryCount < MAX_RETRIES)
    {
        String str = getPayload(tx);
        retryCount++;
        totalRetries++;
        sLog(LORA_TAG, "Retrying... attempt " + String(retryCount));
        sendPacket(tx);
        ackStartTime = millis();
    }
    else
    {
        sLog(LORA_TAG, "Failed to send after " + String(MAX_RETRIES) + " attempts");
        totalFailed++;
        waitingForAck = false;
    }
}

// void sendText(String &str){
//     sequenceCounter++;
//     retryCount = 0;
//     sendPacket(TEXT_TYPE, SOURCE_ID, DESTINATION_ID, sequenceCounter, str);
//     ackStartTime = millis();
//     waitingForAck = true;
//     sLog(LORA_TAG, "Waiting for ACK...");
// }

void receive(){
    
            memset(buffer, 0, sizeof(buffer));
            memset(&rx, 0, sizeof(rx));
            int len = lora.getPacketLength();
            int state = lora.readData(buffer, len);
            sLog(LORA_TAG, "RSSI: " + String(lora.getRSSI()) + " dBm");
            sLog(LORA_TAG, "SNR: " + String(lora.getSNR()) + " dB");
            if (state != RADIOLIB_ERR_NONE)
            {
                sLog(LORA_TAG, "failed, code " + String(state));
                return;
            }
            if (len < 5 || buffer[4] > sizeof(rx.payload))
            {
                sLog(LORA_TAG, "Invalid packet received");
                return;
            }
            fromRaw(buffer, len);
            String str = getPayload(rx);
            if (rx.type == ACK_TYPE)
            {
                sLog(LORA_TAG, "Received ACK! Sequence " + String(rx.sequence));
                totalAcks++;
                waitingForAck = false;
                txChar->setValue(buffer,len);
                txChar->notify();


            }
            else
            {
                if (rx.sequence != lastSequence)
                {
                    lastSequence = rx.sequence;
                    ingoingQueue.push(rx);
                    sLog(LORA_TAG, "Buffered new TEXT: " + str);
                }
                else
                {
                    sLog(LORA_TAG, "Duplicate TEXT received, sequence " + String(rx.sequence));
                }

                sendAck();
            }
}