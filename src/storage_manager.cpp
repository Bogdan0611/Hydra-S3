#include "storage_manager.h"
#include "config.h"
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

bool StorageManager::begin() {
    // cardul are nevoie de putin timp dupa ce primeste curent inainte
    // sa raspunda corect - in testul izolat (doar SD, nimic altceva)
    // aveam un delay(1000) inainte de SD.begin() si mergea perfect; in
    // firmware-ul complet (I2C + ecran + SPI, toate rapid unul dupa
    // altul) fara pauza asta cardul pica. probabil contactul din slot
    // e cam pe muchie, si timpul de stabilizare chiar conteaza
    delay(300);

    // viteza implicita (4MHz) parea prea mare pe firele lungi de
    // breadboard, o cobor la 1MHz - la fel ca la NRF24
    ready = SD.begin(PIN_SD_CS, SPI, 1000000);

    // daca a picat prima incercare, mai incerc de cateva ori cu o mica
    // pauza intre - contactul fiind pe muchie, uneori a doua incercare
    // prinde cardul bine chiar daca prima nu a mers
    int attempt = 0;
    while (!ready && attempt < 12) {
        delay(250);
        ready = SD.begin(PIN_SD_CS, SPI, 1000000);
        attempt++;
    }

    if (ready) {
        Serial.print("[OK] Card SD initializat");
        if (attempt > 0) {
            Serial.print(" (dupa ");
            Serial.print(attempt);
            Serial.print(attempt == 1 ? " incercare)" : " incercari)");
        }
        Serial.println();
    } else {
        Serial.println("[EROARE] Cardul SD nu a fost gasit! Verifica daca e bagat corect si firele SPI.");
    }

    return ready;
}
