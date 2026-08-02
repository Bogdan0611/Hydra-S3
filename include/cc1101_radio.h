#pragma once
#include "SmartRC_CC1101.h"

// tot ce tine de configurarea CC1101 pentru Sub-GHz, ascuns aici ca
// SubGhzApp sa nu trebuiasca sa stie ce inseamna registre ca IOCFG0

class Cc1101Radio {
public:
    // seteaza pinii si porneste cipul. false daca nu il gaseste
    // (fire gresite sau modul defect/nealimentat)
    bool begin();

    // pune cipul in mod "serial asincron" pe frecventa data (MHz) -
    // GDO0 reflecta in timp real semnalul RF demodulat, gata de citit
    // cu un interrupt, la fel ca la un modul RX simplu
    void startListening(float frequencyMHz);

    void stopListening();

    // puterea semnalului receptionat, in dBm
    int getSignalStrength();

private:
    SmartRC_CC1101 chip;
};
