#include <Arduino.h>
#include <TFE_Bibliotheque.h>

const int BitA = 15;
const int BitB = 4;
const int BitC = 5; 
const int BitD = 2;
const int BitLatch = 18;

TFE_Bibliotheque monAfficheur(BitA, BitB, BitC, BitD, BitLatch); //initialisation des pins

void setup() {  
}

void loop() {
  int chiffre = random(9);
  monAfficheur.segment(chiffre); // Affichage
  delay(1000);
}
