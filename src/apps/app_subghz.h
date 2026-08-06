#pragma once
#include "app_interface.h"
#include "display_manager.h"
#include "storage_manager.h"
#include "cc1101_radio.h"
#include "raw_capture.h"

#define SUBGHZ_ITEM_COUNT 5

// frecventa implicita, cea mai comuna la telecomenzi ieftine in Europa
#define SUBGHZ_DEFAULT_MHZ 433.92

// cate semnale salvate afisez in lista - fara scroll, tin simplu,
// incap exact 5 randuri pe ecranul de 64px
#define SUBGHZ_MAX_SAVED_SIGNALS 5

// aplicatia Sub-GHz. are propriul meniu intern cu 5 functii - "Citire
// RAW" si "Semnale salvate" sunt functionale, restul "coming soon".
//
// "Semnale salvate" e doar rasfoire acum (vezi ce fisiere ai pe SD) -
// redarea (Replay/TX) e o bucata separata, mai mare, de implementat
// mai tarziu

enum class SubGhzScreen {
    LIST,
    RAW_LISTENING,
    SAVED_LIST,
    COMING_SOON
};

struct SavedSignalEntry {
    char filename[20];
};

class SubGhzApp : public App {
public:
    SubGhzApp(DisplayManager* disp, StorageManager* storageManager);

    void onEnter() override;
    void onExit() override;
    void onLoop() override;
    void onInput(InputEvent event) override;
    const char* getName() override { return "Sub-GHz (CC1101)"; }

    // BACK inchide toata aplicatia doar din LIST - din celelalte
    // ecrane trebuie sa te intoarca la LIST, nu direct la meniul principal
    bool isAtTopLevel() override { return currentScreen == SubGhzScreen::LIST; }

private:
    DisplayManager* display;
    StorageManager* storage;
    Cc1101Radio radio;
    RawCapture rawCapture;

    SubGhzScreen currentScreen = SubGhzScreen::LIST;
    int selectedIndex = 0;

    // daca CC1101 nu a fost gasit la intrare, blochez functiile care au
    // nevoie de el
    bool radioReady = false;

    // pentru nume unice (raw_0.txt, raw_1.txt, ...) - pornesc de la
    // cate fisiere raw_ exista deja pe SD, nu de la 0, altfel as
    // suprascrie capturi salvate intr-o sesiune anterioara
    int savedFileCount = 0;

    SavedSignalEntry savedSignals[SUBGHZ_MAX_SAVED_SIGNALS];
    int savedSignalCount = 0;
    int savedListIndex = 0;

    void drawList();
    void drawRawListening();
    void drawSavedList();
    void drawComingSoon();

    void handleListInput(InputEvent event);
    void handleRawListeningInput(InputEvent event);
    void handleSavedListInput(InputEvent event);
    void handleComingSoonInput(InputEvent event);

    void startRawCapture();
    void stopRawCapture();
    void saveRawCaptureToSd();

    int countExistingRawFiles();
    void loadSavedSignals();
};
