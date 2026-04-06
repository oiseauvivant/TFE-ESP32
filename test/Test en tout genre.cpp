#include <Arduino.h>

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

void setup()
{
    pinMode(recepteurIR38, INPUT);
    pinMode(recepteurIR56, INPUT);
    pinMode(ledPassage, OUTPUT);

    // pinMode(led38, OUTPUT);
    // pinMode(led56, OUTPUT);

    // PWM 38 kHz sur canal 0
    ledcSetup(0, 38000, 8);
    ledcAttachPin(emetteurIR38, 0);

    // PWM 56 kHz sur canal 1
    ledcSetup(1, 56000, 8);
    ledcAttachPin(emetteurIR56, 1);
}

void loop()
{
    // ---------------- BURST 38 kHz ----------------
    burstIR38();
    // ---------------- BURST 56 kHz ----------------
    burstIR56();
    // ---------------- DÉTECTION 38 kHz ----------------
    detectionSignal38();
    // ---------------- DÉTECTION 56 kHz ----------------
    detectionSignal56();
    // ---------------- LOGIQUE DE PASSAGE ----------------
    logiquePassage();
    // ---------------- LEDS ----------------
    // digitalWrite(led38, signalPresent38 ? HIGH : LOW);
    // digitalWrite(led56, signalPresent56 ? HIGH : LOW);
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
        if (nowMicros - previousMicros38 >= 150)
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
            etat = 0;
        }
        else if (s56)
        {
            etat = 0; // incohérent → annulation
        }
        break;
    }
}