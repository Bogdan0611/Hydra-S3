#pragma once
#include "app_interface.h"
#include "display_manager.h"
#include "storage_manager.h"
#include <Adafruit_PN532.h>

#define NFC_ITEM_COUNT 4

// pauza intre 2 incercari de citire card, in ms - ca sa nu tin antena RF
// aproape permanent activa cat astept un card (consuma curent constant)
#define NFC_POLL_INTERVAL_MS 400

// cate carduri salvate afisez in lista - fara scroll, tin simplu,
// incap exact 5 randuri pe ecranul de 64px
#define NFC_MAX_SAVED_CARDS 5

// aplicatia NFC/RFID, acelasi tipar ca SubGhzApp: meniu intern cu
// cateva functii. "Read Card" si "Saved Cards" sunt functionale.
//
// emularea completa (fara card fizic) nu e realista cu PN532 pentru
// Mifare Classic, de-asta "Write to Magic Card" ramane inca neimplementata -
// fluxul real e citire -> scriere pe un card "magic"

enum class NfcScreen {
    LIST,
    READ_CARD,
    SAVED_LIST,
    COMING_SOON
};

// un card salvat pe SD - doar UID-ul, atat cat ne trebuie sa-l afisam
// (scrierea pe magic card nu e implementata inca)
struct SavedNfcCard {
    uint8_t uid[7];
    uint8_t uidLength;
};

class NfcApp : public App {
public:
    NfcApp(DisplayManager* disp, StorageManager* storageManager);

    void onEnter() override;
    void onExit() override;
    void onLoop() override;
    void onInput(InputEvent event) override;
    const char* getName() override { return "NFC / RFID"; }

    bool isAtTopLevel() override { return currentScreen == NfcScreen::LIST; }

private:
    DisplayManager* display;
    StorageManager* storage;
    Adafruit_PN532 nfc;

    NfcScreen currentScreen = NfcScreen::LIST;
    int selectedIndex = 0;

    bool readerReady = false;

    // cardul gasit la ultima citire, ramane pe ecran pana cer o citire
    // noua (OK) sau ies (SET)
    bool cardFound = false;
    uint8_t uid[7];
    uint8_t uidLength = 0;

    // tin minte cand am incercat ultima oara sa citesc un card, pentru
    // pauza dintre polluri (vezi NFC_POLL_INTERVAL_MS)
    unsigned long lastPollTime = 0;

    SavedNfcCard savedCards[NFC_MAX_SAVED_CARDS];
    int savedCardCount = 0;
    int savedListIndex = 0;

    void drawList();
    void drawReadCard();
    void drawSavedList();
    void drawComingSoon();

    void handleListInput(InputEvent event);
    void handleReadCardInput(InputEvent event);
    void handleSavedListInput(InputEvent event);
    void handleComingSoonInput(InputEvent event);

    void tryReadCard();
    void saveCurrentCard();
    void loadSavedCards();
};
