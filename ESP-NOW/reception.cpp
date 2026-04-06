#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// structure de données reçue
typedef struct struct_message
{
    int id;
    char sens[4];
} struct_message;

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    struct_message incoming;
    memcpy(&incoming, data, sizeof(incoming));

    Serial.print("Reçu de ");
    Serial.print(macStr);
    Serial.print("  id=");
    Serial.println(incoming.id);
    Serial.print("  Sens=");
    Serial.println(incoming.sens);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Erreur: esp_now_init() failed");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);

    Serial.println("Récepteur ESP-NOW prêt");
}

void loop()
{
    // rien à gérer dans loop
    delay(10);
}