#include "cc1101_radio.h"
#include "config.h"

bool Cc1101Radio::begin() {
    chip.setSpiPin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_CC1101_CSN);
    chip.setGDO(PIN_CC1101_GDO0, PIN_CC1101_GDO2);

    chip.Init();

    // getCC1101() citeste un registru cunoscut din cip si verifica
    // raspunsul - daca da false, cablare gresita sau modul nealimentat
    return chip.getCC1101();
}

void Cc1101Radio::startListening(float frequencyMHz) {
    // modulatia 2 = ASK/OOK, folosita de majoritatea telecomenzilor
    // ieftine (porti, alarme, sonerii)
    chip.setModulation(2);
    chip.setMHZ(frequencyMHz);

    // pktFormat 3 = mod serial asincron. cipul nu mai incearca sa
    // interpreteze un pachet cu antet/CRC, doar scoate direct pe GDO0
    // semnalul brut asa cum l-a primit din aer
    chip.setPktFormat(3);

    // IOCFG0 = 0x0D face din GDO0 iesire de date seriala - pinul
    // urmareste in timp real forma semnalului. am gasit combinatia asta
    // in datasheet + niste postari de pe forumuri, pktFormat singur nu
    // era de ajuns
    chip.SpiWriteReg(CC1101_IOCFG0, 0x0D);

    chip.SetRx();
}

void Cc1101Radio::stopListening() {
    chip.setSidle();
}

int Cc1101Radio::getSignalStrength() {
    return chip.getRssi();
}
