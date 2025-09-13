#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

uint8_t START_BYTE = 0x7E;
uint8_t TEXT = 0x01;
uint8_t ACK = 0x02;
uint8_t MY_ID = 0x01;
struct __attribute__((packed)) Packet
{
  uint8_t start;
  uint8_t type;
  uint8_t source;
  uint8_t destination;
  uint8_t sequence;
  uint8_t length;
  uint8_t payload[200];
  uint8_t crc;
};
String myString = "hello from node" + String(MY_ID);
Packet tx;
Packet rx;
int state;
SX1262 lora = new Module(SS, DIO0, RST_LoRa, BUSY_LoRa);

uint16_t crc16(const uint8_t *data, size_t length)
{
  uint16_t crc = 0xFFFF; // Initial value
  uint16_t polynomial = 0x1021;

  for (size_t i = 0; i < length; i++)
  {
    crc ^= (data[i] << 8);
    for (uint8_t j = 0; j < 8; j++)
    {
      if (crc & 0x8000)
      {
        crc = (crc << 1) ^ polynomial;
      }
      else
      {
        crc <<= 1;
      }
    }
  }
  return crc;
}
void fillCRC(Packet &pkt)
{
  // Compute CRC on all fields except CRC itself
  uint8_t temp[260];
  temp[0] = pkt.type;
  temp[1] = pkt.source;
  temp[2] = pkt.destination;
  temp[3] = pkt.sequence;
  temp[4] = pkt.length;

  for (int i = 0; i < pkt.length; i++)
    temp[5 + i] = pkt.payload[i];

  pkt.crc = crc16(temp, 5 + pkt.length);
}
bool verifyCRC(Packet &pkt)
{
  uint8_t temp[260];
  temp[0] = pkt.type;
  temp[1] = pkt.source;
  temp[2] = pkt.destination;
  temp[3] = pkt.sequence;
  temp[4] = pkt.length;

  for (int i = 0; i < pkt.length; i++)
    temp[5 + i] = pkt.payload[i];

  uint16_t calc = crc16(temp, 5 + pkt.length);
  return (calc == pkt.crc);
}

String getPayload(uint8_t payload[], int length)
{
  String string;
  for (int i = 0; i < length; i++)
  {
    string = string + (char)payload[i];
  }
  return string;
}
void setPayload(Packet &packet, String &string)
{
  for (int i = 0; i < string.length(); i++)
  {
    packet.payload[i] = string.charAt(i);
  }
}
void setPacket(Packet &packet, uint8_t type, uint8_t destination, uint8_t sequence, String &payload)
{
  packet.start = START_BYTE;
  packet.type = type;
  packet.source = MY_ID;
  packet.destination = destination;
  packet.sequence = sequence;
  packet.length = payload.length();
  setPayload(packet, payload);
  packet.crc = 0x33;
}

void sendTextPacket(Packet &packet, String &myString)
{

  setPacket(packet, TEXT, 2, 1, myString);
  state = lora.transmit((uint8_t *)&packet, 6+packet.length+1);
  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(
        "\n------------------------------------------------\n"
        "Sent packet! : \n"
        "  [START_BYTE] : " +
        String(packet.start) + "\n"
                               "  [TYPE] : " +
        String(packet.type) + "\n"
                              "  [SOURCE] : " +
        String(packet.source) + "\n"
                                "  [DESTINATION] : " +
        String(packet.destination) + "\n"
                                     "  [SEQUENCE] : " +
        String(packet.sequence) + "\n"
                                  "  [LENGTH] : " +
        String(packet.length) + "\n"
                                "  [PAYLOAD] : " +
        getPayload(packet.payload, packet.length) + "\n"
                                                    "  [CRC] : " +
        String(packet.crc,HEX));
  }
  else
  {
    Serial.print("Transmit failed, code : ");
    Serial.println(state);
  }
}
void receiveTextPacket(Packet &packet)
{
  state = lora.receive((uint8_t *)&packet, sizeof(Packet));
  if (state == RADIOLIB_ERR_NONE )
  {
    Serial.println(
        "\n------------------------------------------------\n"
        "Received packet! : \n"
        "  [START_BYTE] : " +
        String(packet.start) + "\n"
                               "  [TYPE] : " +
        String(packet.type) + "\n"
                              "  [SOURCE] : " +
        String(packet.source) + "\n"
                                "  [DESTINATION] : " +
        String(packet.destination) + "\n"
                                     "  [SEQUENCE] : " +
        String(packet.sequence) + "\n"
                                  "  [LENGTH] : " +
        String(packet.length) + "\n"
                                "  [PAYLOAD] : " +
        getPayload(packet.payload, packet.length) + "\n"
                                                    "  [CRC] : " +
        String(packet.crc,HEX));
  }
}
void setup()
{
  Serial.begin(115200);

  state = lora.begin(433.0, 125.0, 7, 5);

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println("LoRa initialized successfully!");
  }
  else
  {
    Serial.print("LoRa initialization failed, code : ");
    Serial.print(state);
    while (true)
      ;
  }
}
long previousMillis = 0;

void loop()
{
  if (millis() - previousMillis >= 5000)
  {
    sendTextPacket(tx, myString);
    previousMillis = millis();
  }
  receiveTextPacket(rx);
}
