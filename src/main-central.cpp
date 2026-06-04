#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <TFE_Bibliotheque.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

#define TFT_CS 2
#define TFT_RST 15
#define TFT_DC 4

#define btnMode 5
#define btnAjout 34
#define btnRetirer 35

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

Demux demuxout(26, 27, 14, 12, 13); // Pins pour le démultiplexeur de sortie
Demux demuxin(21, 22, 32, 33, 25);  // Pins pour le démultiplexeur d'entrée

// structure de données reçue par ESP-NOW
typedef struct struct_message
{
    int id = 0;
    int sens = 0;
} struct_message;

struct_message dataRecu;

struct totalPortes
{
    int porte[10] = {0};
};

totalPortes PortesEntree;
totalPortes PortesSortie;

// Variables globales
bool PCconnexion = 0;
int mode = 0;
int personnePresente = 0;
int totalEntree = 0;
int totalSortie = 0;

void changementMode();
void majAffichage();

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    memcpy(&dataRecu, data, sizeof(dataRecu));

    if (dataRecu.sens == 1)
    {
        personnePresente++;
        totalEntree++;
        PortesEntree.porte[dataRecu.id]++;
    }
    else if (dataRecu.sens == 0)
    {
        if (personnePresente > 0) // On s'assure de ne pas avoir un nombre négatif de personnes présentes
        {
            personnePresente--;
        }
        totalSortie++;
        PortesSortie.porte[dataRecu.id]++;
    }
}

void setup()
{
    pinMode(btnAjout, INPUT);
    pinMode(btnRetirer, INPUT);
    pinMode(btnMode, INPUT);

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

    tft.cp437(true);
    tft.initR(INITR_BLACKTAB);
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(10, 10);         // Position du texte
    tft.setTextColor(ST77XX_BLUE); // Couleur du texte
    tft.setTextSize(1);            // Taille (1 = normal, 2 = x2, etc.)
    tft.setRotation(3);
}

void loop()
{
    if (Serial.available() > 0)
    {
        String mspPC = Serial.readStringUntil('\n');
        if (mspPC == "PING")
        {
            PCconnexion = 1;
            Serial.println("PONG");
        }
        else if (mspPC == "DECONNEXION")
        {
            PCconnexion = 0;
            tft.setCursor(10, 40);
            tft.print("PC déconnecté");
        }
    }

    changementMode();
    majAffichage();
}

void changementMode()
{
    static unsigned long tDebutAppui = 0;
    static bool etatPrecedent = 0;
    static bool appuiEnCours = 0;

    bool etatActuel = digitalRead(btnMode);
    unsigned long tMaintenant = millis();

    // Début d'appui
    if (etatActuel == 1 && etatPrecedent == 0)
    {
        tDebutAppui = tMaintenant; // on mémorise le moment où l'appui commence
        appuiEnCours = 1;          // on indique qu'un appui est en cours
    }

    // Si le bouton est resté appuyé 10 millisecondes ou plus, on considère l'appui
    if (etatActuel == 1 && (tMaintenant - tDebutAppui) >= 10 && appuiEnCours)
    {

        if (mode >= 3)
        {
            mode = 0;
        }
        else
        {
            mode++;
        }
        tft.fillScreen(ST77XX_BLACK); // Efface l'écran à chaque changement de mode

        appuiEnCours = 0; // on indique que l'appui a été traité
    }

    // Mise à jour de l'état précédent
    etatPrecedent = etatActuel;
}

void majAffichage()
{
    if (mode == 0)
    {
        tft.setTextSize(2);
        tft.setCursor(10, 10);
        tft.println("Personnes");
        tft.setCursor(10, 26);
        tft.print("pr");
        tft.write(0x82); // Affiche le caractère 'é' à partir de la table de caractères CP437
        tft.print("sentes:");
        tft.fillRect(130, 26, 22, 15, ST77XX_BLACK); // Efface la partie du nombre de personnes présentes
        tft.println(personnePresente);
    }
    else if (mode == 1)
    {
        tft.setTextSize(1);

        tft.setCursor(10, 10);
        tft.print("Personnes pr");
        tft.write(0x82); // Affiche le caractère 'é' à partir de la table de caractères CP437
        tft.print("sentes : ");
        tft.fillRect(142, 10, 12, 8, ST77XX_BLACK); // Efface la partie du nombre de personnes présentes
        tft.println(personnePresente);

        tft.setCursor(10, 60);
        tft.print("Total entr");
        tft.write(0x82); // Affiche le caractère 'é' à partir de la table de caractères CP437
        tft.print("es : ");
        tft.fillRect(106, 60, 24, 8, ST77XX_BLACK); // Efface la partie du nombre total d'entrées
        tft.println(totalEntree);

        tft.setCursor(10, 110);
        tft.print("Total sorties : ");
        tft.fillRect(106, 110, 24, 8, ST77XX_BLACK); // Efface la partie du nombre total de sorties
        tft.println(totalSortie);
    }
    else if (mode == 2)
    {
        tft.setTextSize(1);
        for (int i = 0; i < 10; i++)
        {
            tft.setCursor(10, 10 + i * 11);
            tft.print("Entr");
            tft.write(0x82); // Affiche le caractère 'é' à partir de la table de caractères CP437
            tft.print("es ");
            tft.print("porte ");
            tft.print(i);
            tft.print(" : ");
            tft.fillRect(118, 10 + i * 11, 24, 8, ST77XX_BLACK); // Efface la partie du nombre d'entrées pour cette porte
            tft.println(PortesEntree.porte[i]);
        }
    }
    else if (mode == 3)
    {
        tft.setTextSize(1);
        for (int i = 0; i < 10; i++)
        {
            tft.setCursor(10, 10 + i * 11);
            tft.print("Sorties porte ");
            tft.print(i);
            tft.print(" : ");
            tft.fillRect(118, 10 + i * 11, 24, 8, ST77XX_BLACK); // Efface la partie du nombre de sorties pour cette porte
            tft.println(PortesSortie.porte[i]);
        }
    }
}

void ajoutPersonne()
{
    personnePresente++;
    totalEntree++;
}

void retirerPersonne()
{
    if (personnePresente > 0) // On s'assure de ne pas avoir un nombre négatif de personnes présentes
    {
        personnePresente--;
    }
    totalSortie++;
}