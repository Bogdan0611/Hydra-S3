#include <Arduino.h>
#include "bus_manager.h"
#include "input_manager.h"
#include "display_manager.h"
#include "storage_manager.h"
#include "menu_system.h"
#include "apps/placeholder_app.h"
#include "apps/app_subghz.h"
#include "apps/app_nfc.h"
#include "apps/app_ir.h"

// aici doar asamblez piesele: creez managerii, ii leg intre ei, adaug
// module in meniu. logica reala e in fisierele separate, main.cpp
// tine sa ramana scurt, ca o harta a proiectului

InputManager inputManager;
DisplayManager displayManager;
StorageManager storageManager;
MenuSystem menuSystem;

// Sub-GHz, NFC si IR sunt module reale acum. restul raman placeholder
// pana le implementez, la fel cum au fost si astea la inceput
SubGhzApp appSubGhz(&displayManager, &storageManager);
NfcApp appNfc(&displayManager);
IrApp appIr(&displayManager);
PlaceholderApp appWifi("WiFi Attacks", &displayManager);
PlaceholderApp appBle("BLE Attacks", &displayManager);
PlaceholderApp appNrf24("NRF24 (MouseJack)", &displayManager);

void setup() {
    Serial.begin(115200);

    inputManager.begin();

    // bus-urile trebuie configurate PE PINII MEI inainte sa initializez
    // orice modul care le foloseste (ecran, SD), altfel ESP32 asculta
    // pe pinii impliciti, nu pe cei cablati de mine
    BusManager::beginI2C();
    BusManager::beginSPI();

    // daca ecranul nu porneste, nu are rost sa continui - nu s-ar vedea
    // nimic oricum
    if (!displayManager.begin()) {
        while (true) {
            delay(1000);
        }
    }

    // SD e optional la pornire - daca lipseste, meniul tot functioneaza,
    // doar modulele care chiar scriu pe el verifica singure isReady()
    storageManager.begin();

    menuSystem.begin(&displayManager);

    // ordinea de aici = ordinea in care apar in meniu, de sus in jos
    menuSystem.addItem("Sub-GHz (CC1101)", &appSubGhz);
    menuSystem.addItem("NFC / RFID (PN532)", &appNfc);
    menuSystem.addItem("Infrared (IR)", &appIr);
    menuSystem.addItem("WiFi Attacks", &appWifi);
    menuSystem.addItem("BLE Attacks", &appBle);
    menuSystem.addItem("NRF24 (MouseJack)", &appNrf24);

    Serial.println("Sistem pornit. Astept input...");
}

void loop() {
    InputEvent event = inputManager.poll();
    menuSystem.update(event);

    // pauza mica, ochiul nu observa diferenta sub 20ms si nu incarc CPU-ul degeaba
    delay(20);
}
