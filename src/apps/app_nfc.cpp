#include "apps/app_nfc.h"
#include "config.h"
#include <Arduino.h>
#include <string.h>

static const char* NFC_ITEM_NAMES[NFC_ITEM_COUNT] = {
    "Read Card",
    "Write to Magic Card",
    "Saved Cards",
    "Settings"
};

NfcApp::NfcApp(DisplayManager* disp)
    : display(disp), nfc(PIN_PN532_IRQ, PIN_PN532_RESET_NECONECTAT) {}

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
    // timeout mic (50ms) ca sa nu blochez meniul cat astept un card -
    // incerc din nou la fiecare bucla, nu astept la infinit intr-un
    // singur apel
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
    display->drawText(4, 28, uidLine1);

    int nextLineY = 40;
    if (uidLength > 4) {
        char uidLine2[24];
        written = 0;
        for (int i = 4; i < uidLength; i++) {
            written += sprintf(uidLine2 + written, "%02X ", uid[i]);
        }
        display->drawText(4, 40, uidLine2);
        nextLineY = 52;
    }

    char lengthText[24];
    sprintf(lengthText, "Length: %d bytes", uidLength);
    display->drawText(4, nextLineY, lengthText);

    display->drawText(4, 60, "OK=again  SET=back");

    display->sendToScreen();
}

void NfcApp::handleReadCardInput(InputEvent event) {
    if (event == InputEvent::OK && cardFound) {
        cardFound = false;
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
