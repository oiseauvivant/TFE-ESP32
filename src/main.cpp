#include <Arduino.h>
#include <TFE_Bibliotheque.h>
#include <esp_now.h>
#include <WiFi.h>

const int BitA = 13;
const int BitB = 12;
const int BitC = 14;
const int BitD = 27;
const int BitLatch = 26;

int ID = 0;
int changeID = 0;

void burstIR38();
void burstIR56();
void detectionSignal38();
void detectionSignal56();
void logiquePassage();

// --- 38 kHz ---
const int emetteurIR38 = 2;
const int recepteurIR38 = 4;
// const int led38 = 21;

unsigned long previousMicros38 = 0;
bool pwmActif38 = false;

unsigned long lastReceptionMillis38 = 0;
bool signalPresent38 = false;

// --- 56 kHz ---
const int emetteurIR56 = 15;
const int recepteurIR56 = 5;
// const int led56 = 19;

unsigned long previousMicros56 = 0;
bool pwmActif56 = false;

unsigned long lastReceptionMillis56 = 0;
bool signalPresent56 = false;

// --- Logique de passage ---
int etat = 0;
int ledPassage = 21; // LED pour indiquer le passage
int ledEnvoi = 19;   // LED pour indiquer l'envoi du message

// Structure pour les données ESP-NOW
typedef struct struct_message
{
  int id;
  char sens[4];
} struct_message;

// Adresse MAC du récepteur (à remplacer par l'adresse réelle de l'ESP32 récepteur)
uint8_t broadcastAddress[] = {0x00, 0x4b, 0x12, 0x9b, 0xe5, 0x18};

TFE_Bibliotheque monAfficheur(BitA, BitB, BitC, BitD, BitLatch); // initialisation des pins pour l'afficheur 7 segments

// Callback quand les données sont envoyées
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nDernier Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup()
{
  Serial.begin(115200);

  pinMode(recepteurIR38, INPUT);
  pinMode(recepteurIR56, INPUT);
  pinMode(ledPassage, OUTPUT);
  pinMode(ledEnvoi, OUTPUT);

  monAfficheur.segment(ID); // Affichage

  // pinMode(led38, OUTPUT);
  // pinMode(led56, OUTPUT);

  // PWM 38 kHz sur canal 0
  ledcSetup(0, 38000, 8);
  ledcAttachPin(emetteurIR38, 0);

  // PWM 56 kHz sur canal 1
  ledcSetup(1, 56000, 8);
  ledcAttachPin(emetteurIR56, 1);

  pinMode(19, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(18, INPUT);

  // Initialiser WiFi
  WiFi.mode(WIFI_STA);

  // Initialiser ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Erreur initialisation ESP-NOW");
    return;
  }

  // Enregistrer la callback
  esp_now_register_send_cb(OnDataSent);

  // Enregistrer le peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Erreur ajout peer");
    return;
  }
}

void loop()
{

  // ---------------- BURST 38 kHz ----------------
  burstIR38();
  // ---------------- DÉTECTION 38 kHz ----------------
  detectionSignal38();
  // ---------------- BURST 56 kHz ----------------
  burstIR56();
  // ---------------- DÉTECTION 56 kHz ----------------
  detectionSignal56();
  // ---------------- LOGIQUE DE PASSAGE ----------------
  logiquePassage();
  // ---------------- LEDS ----------------
  // digitalWrite(led38, signalPresent38 ? HIGH : LOW);
  // digitalWrite(led56, signalPresent56 ? HIGH : LOW);

  if (digitalRead(18) == HIGH) // Si le bouton est appuyé
  {
    unsigned long t0 = millis();
    while (digitalRead(18) == HIGH)
    {
      if (millis() - t0 > 800) // Si le bouton est maintenu pendant plus de 800ms
      {
        break;
      }
    }
    unsigned long duree = millis() - t0;

    if (duree < 800 && changeID == 1) // appui court
    {
      ID++;
      if (ID > 9)
      {
        ID = 0;
      }
      monAfficheur.segment(ID); // Affichage
    }
    else if (duree >= 800) // appui long
    {
      if (changeID == 0)
      {
        digitalWrite(19, HIGH);
        digitalWrite(21, HIGH);
        changeID = 1;
      }
      else if (changeID == 1)
      {
        digitalWrite(19, LOW);
        digitalWrite(21, LOW);
        changeID = 0;
      }
    }
    delay(250);
  }
}

void burstIR38()
{
  unsigned long nowMicros = micros();

  if (pwmActif38)
  {
    if (nowMicros - previousMicros38 >= 600)
    {
      ledcWrite(0, 0); // OFF
      pwmActif38 = false;
      previousMicros38 = nowMicros;
    }
  }
  else
  {
    if (nowMicros - previousMicros38 >= 600)
    {
      ledcWrite(0, 128); // ON
      pwmActif38 = true;
      previousMicros38 = nowMicros;
    }
  }
}

void burstIR56()
{
  unsigned long nowMicros = micros();
  if (pwmActif56)
  {
    if (nowMicros - previousMicros56 >= 600)
    {
      ledcWrite(1, 0); // OFF
      pwmActif56 = false;
      previousMicros56 = nowMicros;
    }
  }
  else
  {
    if (nowMicros - previousMicros56 >= 600)
    {
      ledcWrite(1, 128); // ON
      pwmActif56 = true;
      previousMicros56 = nowMicros;
    }
  }
}

void detectionSignal38()
{
  unsigned long nowMillis = millis();
  int etat38 = digitalRead(recepteurIR38);

  if (etat38 == LOW)
  {
    lastReceptionMillis38 = nowMillis;
    signalPresent38 = true;
  }
  else
  {
    if (nowMillis - lastReceptionMillis38 > 150)
    {
      signalPresent38 = false;
    }
  }
}

void detectionSignal56()
{
  unsigned long nowMillis = millis();
  int etat56 = digitalRead(recepteurIR56);

  if (etat56 == LOW)
  {
    lastReceptionMillis56 = nowMillis;
    signalPresent56 = true;
  }
  else
  {
    if (nowMillis - lastReceptionMillis56 > 50)
    {
      signalPresent56 = false;
    }
  }
}

void logiquePassage()
{
  bool s38 = !signalPresent38;
  bool s56 = !signalPresent56;

  switch (etat)
  {
  case 0: // zone libre
    if (s38 && !s56)
      etat = 1; // 38 coupé en premier
    else if (s56 && !s38)
      etat = 4; // 56 coupé en premier
    break;

  // --- Sens 38 -> 56 ---
  case 1: // 38 seul coupé
    if (!s38 && !s56)
      etat = 0; // recul, annulation
    else if (s38 && s56)
      etat = 2; // les deux coupés
    break;

  case 2: // 38 & 56 coupés
    if (!s38 && s56)
      etat = 3; // 38 libéré, 56 encore coupé → séquence correcte
    else if (!s38 && !s56)
      etat = 0; // libération simultanée → annulation
    else if (s38 && !s56)
      etat = 1; // retour vers 38 seul → on considère que ça repart, on reste dans ce sens
    break;

  case 3: // 38 libéré, 56 encore coupé
    if (!s38 && !s56)
    {
      // Séquence complète : 38 coupé → 2 coupés → 38 libre → 56 libre
      digitalWrite(ledPassage, HIGH); // 56 -> 38
      delay(10);
      digitalWrite(ledPassage, LOW);

      // Envoyer le message via ESP-NOW
      struct_message myData;
      myData.id = ID;
      strcpy(myData.sens, "out");
      esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
      digitalWrite(ledEnvoi, HIGH);
      delay(10);
      digitalWrite(ledEnvoi, LOW);

      etat = 0;
    }
    // si 38 se recoupe ou autre bazar → on annule
    else if (s38)
    {
      etat = 0;
    }
    break;

  // --- Sens 56 -> 38 ---
  case 4: // 56 seul coupé
    if (!s38 && !s56)
      etat = 0; // recul
    else if (s38 && s56)
      etat = 5; // les deux coupés
    break;

  case 5: // 56 & 38 coupés
    if (!s56 && s38)
      etat = 6; // 56 libéré, 38 encore coupé → séquence correcte
    else if (!s38 && !s56)
      etat = 0; // libération simultanée → annulation
    else if (!s56 && s38)
      etat = 4; // retour vers 56 seul
    break;

  case 6: // 56 libéré, 38 encore coupé
    if (!s38 && !s56)
    {
      // Séquence complète : 56 coupé → 2 coupés → 56 libre → 38 libre
      digitalWrite(ledPassage, HIGH); // 56 -> 38
      delay(10);
      digitalWrite(ledPassage, LOW);

      // Envoyer le message via ESP-NOW
      struct_message myData;
      myData.id = ID;
      strcpy(myData.sens, "in");
      esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
      digitalWrite(ledEnvoi, HIGH);
      delay(10);
      digitalWrite(ledEnvoi, LOW);

      etat = 0;
    }
    else if (s56)
    {
      etat = 0; // incohérent → annulation
    }
    break;
  }
}