#include "esp_system.h"
#include <Arduino.h>

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n--- Infos de la puce ESP32 ---");
    Serial.printf("Modèle de puce : %s\n", ESP.getChipModel());
    Serial.printf("Nombre de cœurs CPU : %d\n", ESP.getChipCores());
    Serial.printf("Taille de la Flash : %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
}

void loop() {}