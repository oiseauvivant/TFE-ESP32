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
totalPortes ancienPortesEntree;
totalPortes ancienPortesSortie;

// Variables globales
bool PCconnexion = 0;
bool flagChangementMode = 1;
int mode = 0;
int personnePresente = 0;
int totalEntree = 0;
int totalSortie = 0;
int ancienPersonnePresente = -1;
int ancienTotalEntree = -1;
int ancienTotalSortie = -1;
int porteActive = -1;                      // numéro de porte en cours d’activation
unsigned long tActivation = 0;             // moment où l’activation a commencé
const unsigned long dureeActivation = 200; // durée en ms

bool appuiBouton(int pin);
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

    porteActive = dataRecu.id; // on mémorise la porte qui a été activée
    tActivation = millis();    // on mémorise le moment de l'activation
}

void setup()
{
    pinMode(btnAjout, INPUT);
    pinMode(btnRetirer, INPUT);
    pinMode(btnMode, INPUT);

    for (int i = 0; i < 10; i++)
    {
        ancienPortesEntree.porte[i] = -1;
        ancienPortesSortie.porte[i] = -1;
    }

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

    if (appuiBouton(btnMode))
    {
        if (mode >= 3)
        {
            mode = 0;
        }
        else
        {
            mode++;
        }
        flagChangementMode = 1;       // on indique qu'il y a eu un changement de mode
        tft.fillScreen(ST77XX_BLACK); // Efface l'écran à chaque changement de mode
    }

    if (appuiBouton(btnAjout))
    {
        personnePresente++;
        totalEntree++;
    }

    if (appuiBouton(btnRetirer))
    {
        if (personnePresente > 0)
        {
            personnePresente--;
        }
        totalSortie++;
    }

    if (porteActive != -1)
    {
        if (millis() - tActivation < dureeActivation)
        {
            if (dataRecu.sens == 1)
            {
                demuxin.select(porteActive); // active la porte d'entrée correspondante
            }
            else if (dataRecu.sens == 0)
            {
                demuxout.select(porteActive); // active la porte de sortie correspondante
            }
        }
        else
        {
            demuxin.disable();  // désactive toutes les portes d'entrée
            demuxout.disable(); // désactive toutes les portes de sortie
            porteActive = -1;   // réinitialise la porte active
        }
    }
    majAffichage();
}

bool appuiBouton(int pin)
{
    static unsigned long tDebutAppui[36] = {0};
    static bool etatPrecedent[36] = {0};
    static bool appuiEnCours[36] = {0};

    bool etatActuel = digitalRead(pin);
    unsigned long tMaintenant = millis();

    // Début d'appui
    if (etatActuel == 1 && etatPrecedent[pin] == 0)
    {
        tDebutAppui[pin] = tMaintenant; // on mémorise le moment où l'appui commence
        appuiEnCours[pin] = 1;          // on indique qu'un appui est en cours
    }

    // Si le bouton est resté appuyé 10 millisecondes ou plus, on considère l'appui
    if (etatActuel == 1 && (tMaintenant - tDebutAppui[pin]) >= 10 && appuiEnCours[pin])
    {
        appuiEnCours[pin] = 0; // on indique que l'appui a été traité
        etatPrecedent[pin] = etatActuel;
        return true; // on considère que le bouton a été appuyé
    }

    if (etatActuel == 0)
    {
        appuiEnCours[pin] = 0;
    }
    etatPrecedent[pin] = etatActuel;
    return false; // pas d'appui détecté
}

void majAffichage()
{
    const int posX[5] = {10, 38, 66, 94, 122};
    const int posY[2] = {50, 90};
    const int width = 25;
    const int height = 28;

    if (mode == 0)
    {
        if (personnePresente != ancienPersonnePresente || flagChangementMode)
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

            ancienPersonnePresente = personnePresente;
        }

        if (flagChangementMode)
        {
            // Dessin des 10 carrés
            for (int i = 0; i < 5; i++)
            {
                tft.fillRect(posX[i], posY[0], width, height, (i % 2 == 0) ? ST77XX_MAGENTA : ST77XX_CYAN);
                tft.fillRect(posX[i], posY[1], width, height, (i % 2 == 0) ? ST77XX_CYAN : ST77XX_MAGENTA);

                tft.setCursor(posX[i] + 8, posY[0] + 8);
                tft.setTextColor(ST77XX_BLACK);
                tft.print(i);

                tft.setCursor(posX[i] + 8, posY[1] + 8);
                tft.setTextColor(ST77XX_BLACK);
                tft.print(i + 5);
            }
            tft.setTextColor(ST77XX_BLUE);
        }

        flagChangementMode = 0; // on indique que l'affichage a été mis à jour après un changement de mode
    }
    else if (mode == 1)
    {
        tft.setTextSize(1);

        if (personnePresente != ancienPersonnePresente || flagChangementMode)
        {
            tft.setCursor(10, 10);
            tft.print("Personnes pr");
            tft.write(0x82); // Affiche le caractère 'é' à partir de la table de caractères CP437
            tft.print("sentes : ");
            tft.fillRect(142, 10, 12, 8, ST77XX_BLACK); // Efface la partie du nombre de personnes présentes
            tft.println(personnePresente);

            ancienPersonnePresente = personnePresente;
        }

        if (totalEntree != ancienTotalEntree || flagChangementMode)
        {
            tft.setCursor(10, 60);
            tft.print("Total entr");
            tft.write(0x82); // Affiche le caractère 'é' à partir de la table de caractères CP437
            tft.print("es : ");
            tft.fillRect(106, 60, 24, 8, ST77XX_BLACK); // Efface la partie du nombre total d'entrées
            tft.println(totalEntree);

            ancienTotalEntree = totalEntree;
        }

        if (totalSortie != ancienTotalSortie || flagChangementMode)
        {
            tft.setCursor(10, 110);
            tft.print("Total sorties : ");
            tft.fillRect(106, 110, 24, 8, ST77XX_BLACK); // Efface la partie du nombre total de sorties
            tft.println(totalSortie);

            ancienTotalSortie = totalSortie;
        }

        flagChangementMode = 0;
    }
    else if (mode == 2)
    {
        tft.setTextSize(1);
        for (int i = 0; i < 10; i++)
        {
            if (PortesEntree.porte[i] != ancienPortesEntree.porte[i] || flagChangementMode)
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

                ancienPortesEntree.porte[i] = PortesEntree.porte[i];
            }
        }
        flagChangementMode = 0;
    }
    else if (mode == 3)
    {
        tft.setTextSize(1);
        for (int i = 0; i < 10; i++)
        {
            if (PortesSortie.porte[i] != ancienPortesSortie.porte[i] || flagChangementMode)
            {
                tft.setCursor(10, 10 + i * 11);
                tft.print("Sorties porte ");
                tft.print(i);
                tft.print(" : ");
                tft.fillRect(118, 10 + i * 11, 24, 8, ST77XX_BLACK); // Efface la partie du nombre de sorties pour cette porte
                tft.println(PortesSortie.porte[i]);

                ancienPortesSortie.porte[i] = PortesSortie.porte[i];
            }
        }
        flagChangementMode = 0;
    }
}
