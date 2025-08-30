#include <Arduino.h>
#include <HT_SSD1306Wire.h>
#include "image.h"

SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

#define DEMO_DURATION 3000
typedef void (*Demo)(void);

int demoMode = 0;
int counter = 0;

void drawText()
{
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(128, 64, "size 10");

    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.drawString(display.width() / 2, display.height() / 2 - 8, "size 16");

    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.setFont(ArialMT_Plain_24);
    display.drawString(display.width(), display.height() - 24, "size 24");
}

void drawTextFlow()
{
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawStringMaxWidth(0, 0, 128, "this is a long text to test TextFlow functionality, it should wrap to the next line when it hits the end of the line");
}

void drawRect()
{
    for (int i = 0; i < 7; i++)
    {
        display.setPixel(i, i);
        display.setPixel(7 - i, i);
    }

    display.drawRect(35, 20, 30, 30);
    display.fillRect(75, 20, 30, 30);

    display.drawHorizontalLine(0, 20, 10);
    display.drawVerticalLine(20, 0, 10);
}
void drawCircle()
{
    for (int i = 0; i < 9; i++)
    {
        display.setPixel(i, i);
        display.setPixel(9 - i, i);
    }

    display.drawCircle(display.width() / 3, display.height() / 2, 15);
    display.fillCircle(display.width() * 2 / 3, display.height() / 2, 15);

    display.drawHorizontalLine(0, 15, 10);
    display.drawVerticalLine(15, 0, 10);
}

void drawProgressBar()
{
    int progress = counter % 101;
    display.drawProgressBar(display.width() / 2 - 50, display.height() / 2, 100, 10, progress);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(display.width() / 2, display.height() / 2 + 18, String(progress) + "%");
    counter++;
    delay(250);
}

void drawImage()
{
    display.drawXbm(15, 5, image_width, image_height, image_bits);
}

void VextON(void)
{
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
}
void VextOFF(void)
{
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, HIGH);
}

void setup()
{
    VextON();
    delay(100);

    display.init();
}

Demo demo[] = {drawText, drawTextFlow, drawRect, drawCircle, drawProgressBar, drawImage};
int demoSize = sizeof(demo) / sizeof(Demo);
long timeSinceLastDemo = 0;
void loop()
{
    display.clear();
    demo[demoMode]();
    display.display();

    if (millis() - timeSinceLastDemo > DEMO_DURATION)
    {
        demoMode = (demoMode + 1) % demoSize;
        timeSinceLastDemo = millis();
    }
}
