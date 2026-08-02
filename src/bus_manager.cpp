#include "bus_manager.h"
#include "config.h"
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

void BusManager::beginI2C() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
}

void BusManager::beginSPI() {
    // ultimul parametru (ss) il las -1 pentru ca fiecare modul (CC1101,
    // NRF24, SD) isi controleaza singur propriul CS din codul lui
    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, -1);

    // asta a fost bug-ul mare: pe un bus SPI comun, orice CS lasat
    // neconfigurat "pluteste" pana il seteaza codul modulului respectiv.
    // Cand testam module pe rand, CS-urile celorlalte plutind faceau
    // sa raspunda peste modulul testat si stricau tot. Le dezactivez
    // pe toate (HIGH = deselectat) chiar de la inceput, inainte sa
    // apuce vreunul sa fie folosit singur.
    pinMode(PIN_CC1101_CSN, OUTPUT);
    digitalWrite(PIN_CC1101_CSN, HIGH);

    pinMode(PIN_NRF24_CSN, OUTPUT);
    digitalWrite(PIN_NRF24_CSN, HIGH);

    pinMode(PIN_SD_CS, OUTPUT);
    digitalWrite(PIN_SD_CS, HIGH);
}
