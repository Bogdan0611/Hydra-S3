#include "apps/app_nfc.h"
#include "config.h"
#include <Arduino.h>
#include <SD.h>
#include <string.h>

static const char* NFC_ITEM_NAMES[NFC_ITEM_COUNT] = {
    "Read Card",
    "Write to Magic Card",
    "Saved Cards",
    "Settings"
};

NfcApp::NfcApp(DisplayManager* disp, StorageManager* storageManager)
    : display(disp), storage(storageManager), nfc(PIN_PN532_IRQ, PIN_PN532_RESET_NECONECTAT) {}

void NfcApp::onEnter() {
    nfc.begin();

    // getFirmwareVersion() citeste versiunea firmware-ului scrisa in
    // cip din fabrica - primesc 0 daca nu a raspuns nimic
    uint32_t firmwareVersion = nfc.getFirmwareVersion();
    readerReady = (firmwareVersion != 0);

    if (readerReady) {
        // fara SAMConfig cipul ramane "adormit" si nu raspunde la carduri
        nfc.SAMConfig();
    }

    currentScreen = NfcScreen::LIST;
    selectedIndex = 0;
}

void NfcApp::onExit() {
    // PN532 nu are mod de ascultare continua ca CC1101, fiecare citire
    // e ceruta punctual - nu e nimic de oprit aici
}

void NfcApp::onInput(InputEvent event) {
    switch (currentScreen) {
        case NfcScreen::LIST:
            handleListInput(event);
            break;
        case NfcScreen::READ_CARD:
            handleReadCardInput(event);
            break;
        case NfcScreen::SAVED_LIST:
            handleSavedListInput(event);
            break;
        case NfcScreen::COMING_SOON:
            handleComingSoonInput(event);
            break;
    }
}

void NfcApp::onLoop() {
    switch (currentScreen) {
        case NfcScreen::LIST:
            drawList();
            break;
        case NfcScreen::READ_CARD:
            if (!cardFound) {
                tryReadCard();
            }
            drawReadCard();
            break;
        case NfcScreen::SAVED_LIST:
            drawSavedList();
            break;
        case NfcScreen::COMING_SOON:
            drawComingSoon();
            break;
    }
}

// ------------------------------------------------------------
// ecranul LIST
// ------------------------------------------------------------

void NfcApp::handleListInput(InputEvent event) {
    if (event == InputEvent::UP) {
        selectedIndex = (selectedIndex - 1 + NFC_ITEM_COUNT) % NFC_ITEM_COUNT;
    } else if (event == InputEvent::DOWN) {
        selectedIndex = (selectedIndex + 1) % NFC_ITEM_COUNT;
    } else if (event == InputEvent::OK) {
        if (selectedIndex == 0) {
            cardFound = false;
            currentScreen = NfcScreen::READ_CARD;
        } else if (selectedIndex == 2) {
            loadSavedCards();
            savedListIndex = 0;
            currentScreen = NfcScreen::SAVED_LIST;
        } else {
            currentScreen = NfcScreen::COMING_SOON;
        }
    }
}

void NfcApp::drawList() {
    display->clear();

    if (!readerReady) {
        display->drawText(4, 12, "PN532 not found!");
        display->drawText(4, 28, "Check I2C wiring");
        display->drawText(4, 44, "and mode switches");
        display->sendToScreen();
        return;
    }

    for (int i = 0; i < NFC_ITEM_COUNT; i++) {
        int yPosition = 12 + (i * 12);
        bool isSelected = (i == selectedIndex);

        if (isSelected) {
            display->drawSelectionBox(0, yPosition - 9, 128, 11);
        }

        display->drawText(4, yPosition, NFC_ITEM_NAMES[i], isSelected);
    }

    display->sendToScreen();
}

// ------------------------------------------------------------
// ecranul READ_CARD - citire live de UID
// ------------------------------------------------------------

void NfcApp::tryReadCard() {
    // antena RF a PN532 sta activa cat dureaza un poll - daca incerc la
    // fiecare bucla (la ~20-70ms), antena practic nu se mai opreste
    // niciodata cat esti pe ecranul asta, si trage curent constant.
    // pun o pauza intre incercari ca sa scad consumul mediu, nu doar
    // varfurile - antena sta stinsa majoritatea timpului, nu doar activa
    if (millis() - lastPollTime < NFC_POLL_INTERVAL_MS) {
        return;
    }
    lastPollTime = millis();

    // timeout mic (50ms) ca sa nu blochez meniul cat astept un card
    uint8_t newUid[7];
    uint8_t newUidLength;

    bool found = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, newUid, &newUidLength, 50);

    if (found) {
        memcpy(uid, newUid, newUidLength);
        uidLength = newUidLength;
        cardFound = true;
    }
}

void NfcApp::drawReadCard() {
    display->clear();

    if (!cardFound) {
        display->drawText(4, 12, "Read Card");
        display->drawText(4, 32, "Hold a tag near");
        display->drawText(4, 44, "the antenna...");
        display->drawText(4, 60, "SET=back");
        display->sendToScreen();
        return;
    }

    display->drawText(4, 12, "Card detected!");

    // afisez UID-ul ca octeti in hex, pe maxim 2 randuri - Mifare
    // Classic clasic are 4 octeti, dar unele carduri au 7
    char uidLine1[24];
    int written = 0;
    for (int i = 0; i < uidLength && i < 4; i++) {
        written += sprintf(uidLine1 + written, "%02X ", uid[i]);
    }
    display->drawText(4, 24, uidLine1);

    if (uidLength > 4) {
        char uidLine2[24];
        written = 0;
        for (int i = 4; i < uidLength; i++) {
            written += sprintf(uidLine2 + written, "%02X ", uid[i]);
        }
        display->drawText(4, 34, uidLine2);
    }

    char lengthText[24];
    sprintf(lengthText, "Length: %d bytes", uidLength);
    display->drawText(4, 44, lengthText);

    display->drawText(4, 54, "OK=again  R=save");
    display->drawText(4, 63, "SET=back");

    display->sendToScreen();
}

void NfcApp::handleReadCardInput(InputEvent event) {
    if (event == InputEvent::OK && cardFound) {
        cardFound = false;
    } else if (event == InputEvent::RIGHT && cardFound) {
        saveCurrentCard();
    } else if (event == InputEvent::BACK) {
        currentScreen = NfcScreen::LIST;
    }
}

// ------------------------------------------------------------
// salvarea cardului curent pe SD (buton RIGHT din ecranul READ_CARD)
// ------------------------------------------------------------

void NfcApp::saveCurrentCard() {
    display->clear();

    if (!storage->isReady()) {
        display->drawText(4, 24, "Eroare: SD lipsa!");
        display->sendToScreen();
        delay(1000);
        return;
    }

    // format simplu: UID-ul scris ca text hex lipit, apoi lungimea lui,
    // o linie per card. FILE_APPEND adauga la finalul fisierului
    File file = SD.open("/nfc_cards.txt", FILE_APPEND);
    if (!file) {
        display->drawText(4, 24, "Nu am putut salva");
        display->sendToScreen();
        delay(1000);
        return;
    }

    size_t sizeBefore = file.size();

    for (int i = 0; i < uidLength; i++) {
        char byteText[3];
        sprintf(byteText, "%02X", uid[i]);
        file.print(byteText);
    }
    file.print(' ');
    file.println(uidLength);
    file.close();

    // verific ca fisierul chiar a crescut pe card, nu doar ca file.print()
    // nu a dat eroare - un card mort poate "accepta" scrierea fara sa o retina
    File verify = SD.open("/nfc_cards.txt");
    size_t sizeAfter = verify ? verify.size() : 0;
    verify.close();

    if (sizeAfter <= sizeBefore) {
        Serial.println("[EROARE] nfc_cards.txt nu a crescut dupa scriere - cardul nu retine datele.");
        display->drawText(4, 24, "EROARE: cardul nu");
        display->drawText(4, 40, "retine scrierea!");
        display->sendToScreen();
        delay(1500);
        return;
    }

    display->drawText(4, 24, "Salvat!");
    display->sendToScreen();
    delay(800);
}

// ------------------------------------------------------------
// ecranul SAVED_LIST - cardurile salvate pe SD
// ------------------------------------------------------------

void NfcApp::loadSavedCards() {
    savedCardCount = 0;

    if (!storage->isReady()) {
        return;
    }

    File file = SD.open("/nfc_cards.txt");
    if (!file) {
        // nu exista inca niciun card salvat, e ok, lista ramane goala
        return;
    }

    while (file.available() && savedCardCount < NFC_MAX_SAVED_CARDS) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) {
            continue;
        }

        int spacePos = line.indexOf(' ');
        if (spacePos == -1) {
            continue; // linie stricata, o sar
        }

        String hexPart = line.substring(0, spacePos);
        String lengthPart = line.substring(spacePos + 1);

        int byteCount = hexPart.length() / 2;
        if (byteCount > 7) {
            byteCount = 7;
        }

        for (int i = 0; i < byteCount; i++) {
            String byteStr = hexPart.substring(i * 2, i * 2 + 2);
            savedCards[savedCardCount].uid[i] = (uint8_t)strtoul(byteStr.c_str(), nullptr, 16);
        }
        savedCards[savedCardCount].uidLength = lengthPart.toInt();

        savedCardCount++;
    }

    file.close();
}

void NfcApp::drawSavedList() {
    display->clear();

    if (savedCardCount == 0) {
        display->drawText(4, 24, "No saved cards");
        display->drawText(4, 40, "SET=back");
        display->sendToScreen();
        return;
    }

    for (int i = 0; i < savedCardCount; i++) {
        int yPosition = 12 + (i * 12);
        bool isSelected = (i == savedListIndex);

        if (isSelected) {
            display->drawSelectionBox(0, yPosition - 9, 128, 11);
        }

        char lineText[24];
        int written = snprintf(lineText, sizeof(lineText), "%d. ", i + 1);
        for (int b = 0; b < savedCards[i].uidLength && written < 22; b++) {
            written += snprintf(lineText + written, sizeof(lineText) - written, "%02X", savedCards[i].uid[b]);
        }
        display->drawText(4, yPosition, lineText, isSelected);
    }

    display->sendToScreen();
}

void NfcApp::handleSavedListInput(InputEvent event) {
    if (savedCardCount == 0) {
        if (event == InputEvent::BACK) {
            currentScreen = NfcScreen::LIST;
        }
        return;
    }

    if (event == InputEvent::UP) {
        savedListIndex = (savedListIndex - 1 + savedCardCount) % savedCardCount;
    } else if (event == InputEvent::DOWN) {
        savedListIndex = (savedListIndex + 1) % savedCardCount;
    } else if (event == InputEvent::BACK) {
        currentScreen = NfcScreen::LIST;
    }
}

// ------------------------------------------------------------
// ecranul COMING_SOON
// ------------------------------------------------------------

void NfcApp::drawComingSoon() {
    display->clear();
    display->drawText(4, 20, NFC_ITEM_NAMES[selectedIndex]);
    display->drawText(4, 36, "Coming soon...");
    display->drawText(4, 52, "Press any key");
    display->sendToScreen();
}

void NfcApp::handleComingSoonInput(InputEvent event) {
    if (event != InputEvent::NONE) {
        currentScreen = NfcScreen::LIST;
    }
}
