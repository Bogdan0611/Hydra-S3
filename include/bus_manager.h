#pragma once

// pornirea bus-urilor I2C si SPI, pe pinii mei reali din config.h
// ESP32 nu foloseste automat pinii mei, deci trebuie facut asta INAINTE
// de orice modul care foloseste I2C sau SPI (ecran, SD, etc), altfel
// asculta pe pinii impliciti si nimic nu raspunde

class BusManager {
public:
    // I2C - ecranul OLED si PN532 stau pe acelasi bus
    static void beginI2C();

    // SPI - CC1101, NRF24, SD stau pe acelasi bus (fire comune SCK/MISO/MOSI,
    // fiecare cu propriul CS)
    static void beginSPI();
};
