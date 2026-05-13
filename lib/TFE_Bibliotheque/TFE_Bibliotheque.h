#ifndef TFE_Bibliotheque_h
#define TFE_Bibliotheque_h

#include "Arduino.h"

class TFE_Bibliotheque
{
public:
  TFE_Bibliotheque(int a, int b, int c, int d, int latch);
  void segment(int chiffre);

private:
  int BitA, BitB, BitC, BitD, BitLatch;
};

class Demux
{
public:
  Demux(int a, int b, int c, int d, int enable);
  void select(int canal);

private:
  int BitA, BitB, BitC, BitD, Enable;
};

#endif