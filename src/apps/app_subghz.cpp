#include "apps/app_subghz.h"
#include "config.h"
#include <Arduino.h>
#include <SD.h>

static const char* SUBGHZ_ITEM_NAMES[SUBGHZ_ITEM_COUNT] = {
    "Citire RAW",
    "Citire decodata",
    "Analizor frecvente",
    "Semnale salvate",
    "Setari"
};

SubGhzApp::SubGhzApp(DisplayManager* disp, StorageManager* storageManager)
    : display(disp), storage(storageManager) {}

void SubGhzApp::onEnter() {
    // initializez CC1101 doar aici, cand intru in aplicatie, nu la
    // fiecare apasare de buton
    radioReady = radio.begin();
    currentScreen = SubGhzScreen::LIST;
    selectedIndex = 0;
}

void SubGhzApp::onExit() {
    // nu vreau sa ramana "ascultand" cand ies din aplicatie
    if (currentScreen == SubGhzScreen::RAW_LISTENING) {
        stopRawCapture();
    }
}

void SubGhzApp::onInput(InputEvent event) {
    switch (currentScreen) {
        case SubGhzScreen::LIST:
            handleListInput(event);
            break;
        case SubGhzScreen::RAW_LISTENING:
            handleRawListeningInput(event);
            break;
        case SubGhzScreen::COMING_SOON:
            handleComingSoonInput(event);
            break;
    }
}

void SubGhzApp::onLoop() {
    switch (currentScreen) {
        case SubGhzScreen::LIST:
            drawList();
            break;
        case SubGhzScreen::RAW_LISTENING:
            drawRawListening();
            break;
        case SubGhzScreen::COMING_SOON:
            drawComingSoon();
            break;
    }
}

// ------------------------------------------------------------
// ecranul LIST - lista cu cele 5 functii
// ------------------------------------------------------------

void SubGhzApp::handleListInput(InputEvent event) {
    if (event == InputEvent::UP) {
        selectedIndex = (selectedIndex - 1 + SUBGHZ_ITEM_COUNT) % SUBGHZ_ITEM_COUNT;
    } else if (event == InputEvent::DOWN) {
        selectedIndex = (selectedIndex + 1) % SUBGHZ_ITEM_COUNT;
    } else if (event == InputEvent::OK) {
        if (selectedIndex == 0) {
            startRawCapture();
            currentScreen = SubGhzScreen::RAW_LISTENING;
        } else {
            currentScreen = SubGhzScreen::COMING_SOON;
        }
    }
}

void SubGhzApp::drawList() {
    display->clear();

    if (!radioReady) {
        display->drawText(4, 12, "CC1101 negasit!");
        display->drawText(4, 28, "Verifica cablarea");
        display->drawText(4, 44, "si alimentarea 3.3V");
        display->sendToScreen();
        return;
    }

    for (int i = 0; i < SUBGHZ_ITEM_COUNT; i++) {
        int yPosition = 12 + (i * 12);
        bool isSelected = (i == selectedIndex);

        if (isSelected) {
            display->drawSelectionBox(0, yPosition - 9, 128, 11);
        }

        display->drawText(4, yPosition, SUBGHZ_ITEM_NAMES[i], isSelected);
    }

    display->sendToScreen();
}

// ------------------------------------------------------------
// ecranul RAW_LISTENING - captura live a semnalului
// ------------------------------------------------------------

void SubGhzApp::startRawCapture() {
    rawCapture.begin(PIN_CC1101_GDO0);
    radio.startListening(SUBGHZ_DEFAULT_MHZ);
    rawCapture.start();
}

void SubGhzApp::stopRawCapture() {
    rawCapture.stop();
    radio.stopListening();
}

void SubGhzApp::handleRawListeningInput(InputEvent event) {
    if (event == InputEvent::OK) {
        // OK = opresc si salvez ce am capturat
        stopRawCapture();
        saveRawCaptureToSd();
        currentScreen = SubGhzScreen::LIST;
    } else if (event == InputEvent::BACK) {
        // BACK = renunt fara sa salvez
        stopRawCapture();
        currentScreen = SubGhzScreen::LIST;
    }
}

void SubGhzApp::drawRawListening() {
    display->clear();

    display->drawText(4, 12, "Citire RAW...");

    char sampleCountText[32];
    if (rawCapture.isFull()) {
        sprintf(sampleCountText, "Pulsuri: %d (max)", rawCapture.getSampleCount());
    } else {
        sprintf(sampleCountText, "Pulsuri: %d", rawCapture.getSampleCount());
    }
    display->drawText(4, 24, sampleCountText);

    char rssiText[24];
    sprintf(rssiText, "RSSI: %d dBm", radio.getSignalStrength());
    display->drawText(4, 36, rssiText);

    display->drawText(4, 48, "OK=salveaza");
    display->drawText(4, 60, "SET=renunta");

    display->sendToScreen();
}

void SubGhzApp::saveRawCaptureToSd() {
    if (!storage->isReady()) {
        display->clear();
        display->drawText(4, 24, "Eroare: SD lipsa!");
        display->drawText(4, 40, "Nu am putut salva");
        display->sendToScreen();
        delay(1500);
        return;
    }

    char filename[32];
    sprintf(filename, "/raw_%d.txt", savedFileCount);

    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        Serial.println("[EROARE] Nu am putut deschide fisierul pentru scriere pe SD.");
        return;
    }

    // prima linie = frecventa ascultata in Hz, utila pentru Replay
    file.println((long)(SUBGHZ_DEFAULT_MHZ * 1000000));

    int sampleCount = rawCapture.getSampleCount();
    for (int i = 0; i < sampleCount; i++) {
        file.println(rawCapture.getSample(i));
    }

    file.close();
    savedFileCount++;

    display->clear();
    display->drawText(4, 24, "Salvat!");

    char savedFileText[32];
    sprintf(savedFileText, "Fisier: %s", filename);
    display->drawText(4, 40, savedFileText);
    display->sendToScreen();
    delay(1200);
}

// ------------------------------------------------------------
// ecranul COMING_SOON - pentru functiile neimplementate
// ------------------------------------------------------------

void SubGhzApp::drawComingSoon() {
    display->clear();
    display->drawText(4, 20, SUBGHZ_ITEM_NAMES[selectedIndex]);
    display->drawText(4, 36, "In curand...");
    display->drawText(4, 52, "Apasa orice tasta");
    display->sendToScreen();
}

void SubGhzApp::handleComingSoonInput(InputEvent event) {
    if (event != InputEvent::NONE) {
        currentScreen = SubGhzScreen::LIST;
    }
}
