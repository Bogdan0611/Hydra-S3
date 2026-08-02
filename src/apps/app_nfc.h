#pragma once
#include "app_interface.h"
#include "display_manager.h"
#include <Adafruit_PN532.h>

#define NFC_ITEM_COUNT 4

// aplicatia NFC/RFID, acelasi tipar ca SubGhzApp: meniu intern cu
// cateva functii, doar "Read Card" e functionala acum.
//
// emularea completa (fara card fizic) nu e realista cu PN532 pentru
// Mifare Classic, de-asta "Write to Magic Card" ramane inca neimplementata -
// fluxul real e citire -> scriere pe un card "magic"

enum class NfcScreen {
    LIST,
    READ_CARD,
    COMING_SOON
};

class NfcApp : public App {
public:
    explicit NfcApp(DisplayManager* disp);

    void onEnter() override;
    void onExit() override;
    void onLoop() override;
    void onInput(InputEvent event) override;
    const char* getName() override { return "NFC / RFID"; }

    bool isAtTopLevel() override { return currentScreen == NfcScreen::LIST; }

private:
    DisplayManager* display;
    Adafruit_PN532 nfc;

    NfcScreen currentScreen = NfcScreen::LIST;
    int selectedIndex = 0;

    bool readerReady = false;

    // cardul gasit la ultima citire, ramane pe ecran pana cer o citire
    // noua (OK) sau ies (SET)
    bool cardFound = false;
    uint8_t uid[7];
    uint8_t uidLength = 0;

    void drawList();
    void drawReadCard();
    void drawComingSoon();

    void handleListInput(InputEvent event);
    void handleReadCardInput(InputEvent event);
    void handleComingSoonInput(InputEvent event);

    void tryReadCard();
};
