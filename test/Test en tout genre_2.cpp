#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Structure envoyée
typedef struct struct_message
{
    int id;
    int sens;
} struct_message;

// Adresse MAC du récepteur
uint8_t broadcastAddress[] = {0x00, 0x4B, 0x12, 0x9B, 0xE5, 0x18};

// Callback d’envoi (optionnelle mais utile)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.print("Statut envoi : ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "ECHEC");
}

void setup()
{
    Serial.begin(115200);

    // Mode WiFi obligatoire
    WiFi.mode(WIFI_STA);

    // Initialisation ESP‑NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Erreur ESP-NOW");
        return;
    }

    // Callback d’envoi
    esp_now_register_send_cb(OnDataSent);

    // Déclaration du peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0; // même canal que le WiFi STA
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_STA; // interface correcte

    // Ajout du peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Erreur ajout peer");
        return;
    }

    Serial.println("Émetteur ESP‑NOW prêt");
}

void loop()
{
    int id = random(10);
    int sens = random(2); // 0 ou 1
    // Exemple d’envoi simple
    struct_message msg;
    msg.id = id;     // valeur test
    msg.sens = sens; // valeur test
    esp_now_send(broadcastAddress, (uint8_t *)&msg, sizeof(msg));

    delay(1000); // envoi toutes les secondes
}