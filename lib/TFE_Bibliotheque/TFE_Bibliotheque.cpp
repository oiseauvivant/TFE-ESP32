#include "TFE_Bibliotheque.h"

TFE_Bibliotheque::TFE_Bibliotheque(int a, int b, int c, int d, int latch)
{
  BitA = a;
  BitB = b;
  BitC = c;
  BitD = d;
  BitLatch = latch;

  pinMode(BitA, OUTPUT);
  pinMode(BitB, OUTPUT);
  pinMode(BitC, OUTPUT);
  pinMode(BitD, OUTPUT);
  pinMode(BitLatch, OUTPUT);
}

Demux::Demux(int a, int b, int c, int d, int enable)
{
  BitA = a;
  BitB = b;
  BitC = c;
  BitD = d;
  Enable = enable;

  pinMode(BitA, OUTPUT);
  pinMode(BitB, OUTPUT);
  pinMode(BitC, OUTPUT);
  pinMode(BitD, OUTPUT);
  pinMode(Enable, OUTPUT);
  digitalWrite(Enable, LOW); // Activer le démultiplexeur
}

void TFE_Bibliotheque::segment(int chiffre)
{
  byte bcd[10][4] = {
      {0, 0, 0, 0}, // 0
      {1, 0, 0, 0}, // 1
      {0, 1, 0, 0}, // 2
      {1, 1, 0, 0}, // 3
      {0, 0, 1, 0}, // 4
      {1, 0, 1, 0}, // 5
      {0, 1, 1, 0}, // 6
      {1, 1, 1, 0}, // 7
      {0, 0, 0, 1}, // 8
      {1, 0, 0, 1}  // 9
  };

  digitalWrite(BitLatch, HIGH);
  digitalWrite(BitA, bcd[chiffre][0]);
  digitalWrite(BitB, bcd[chiffre][1]);
  digitalWrite(BitC, bcd[chiffre][2]);
  digitalWrite(BitD, bcd[chiffre][3]);
  digitalWrite(BitLatch, LOW);
}

void Demux::select(int canal)
{
  if (canal < 0 || canal > 15)
    return; // Canal invalide

  digitalWrite(BitA, canal & 0x01);
  digitalWrite(BitB, (canal >> 1) & 0x01);
  digitalWrite(BitC, (canal >> 2) & 0x01);
  digitalWrite(BitD, (canal >> 3) & 0x01);
}