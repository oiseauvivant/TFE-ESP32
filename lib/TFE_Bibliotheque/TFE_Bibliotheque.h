#ifndef TFE_Bibliotheque_h
#define TFE_Bibliotheque_h

#include "Arduino.h"

class TFE_Bibliotheque {
  public:
    TFE_Bibliotheque(int a, int b, int c, int d, int latch);
	void segment(int chiffre);
  private:
    int BitA, BitB, BitC, BitD, BitLatch;
};

#endif