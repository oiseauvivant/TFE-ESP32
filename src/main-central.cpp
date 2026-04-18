#include <Arduino.h>
#include <TFE_Bibliotheque.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define TFT_CS 2
#define TFT_RST 15
#define TFT_DC 4

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

String data;

void setup()
{
    tft.initR(INITR_BLACKTAB);
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(10, 10);         // Position du texte
    tft.setTextColor(ST77XX_BLUE); // Couleur du texte
    tft.setTextSize(1);            // Taille (1 = normal, 2 = x2, etc.)
    tft.setRotation(1);

    Serial.begin(115200);
}

void loop()
{
    /*char id[10];
    sprintf(id, "id=%d", 2);
    char trame[30];
    sprintf(trame, "%s;sens=entree;nbr=1", id);
    Serial.println(trame);

    delay(1000);*/

    if (Serial.available() > 0)
    {
        data = Serial.readStringUntil('\n');
        if (data == "PING")
        {
            Serial.println("PONG");
        }
        tft.print(data);
    }
}