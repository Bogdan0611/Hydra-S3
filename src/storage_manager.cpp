#include "storage_manager.h"
#include "config.h"
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

bool StorageManager::begin() {
    // viteza implicita (4MHz) parea prea mare pe firele lungi de
    // breadboard, o cobor la 1MHz - la fel ca la NRF24
    ready = SD.begin(PIN_SD_CS, SPI, 1000000);

    if (!ready) {
        Serial.println("[EROARE] Cardul SD nu a fost gasit! Verifica daca e bagat corect si firele SPI.");
    }

    return ready;
}
