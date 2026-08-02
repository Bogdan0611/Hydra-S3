#pragma once
#include "app_interface.h"
#include "display_manager.h"
#include "storage_manager.h"
#include "cc1101_radio.h"
#include "raw_capture.h"

#define SUBGHZ_ITEM_COUNT 5

// frecventa implicita, cea mai comuna la telecomenzi ieftine in Europa
#define SUBGHZ_DEFAULT_MHZ 433.92

// aplicatia Sub-GHz. are propriul meniu intern cu 5 functii, doar
// "Citire RAW" e functionala acum, restul arata "coming soon"

enum class SubGhzScreen {
    LIST,
    RAW_LISTENING,
    COMING_SOON
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

    // cate fisiere am salvat in sesiunea asta, pentru nume unice (raw_0.txt, ...)
    int savedFileCount = 0;

    void drawList();
    void drawRawListening();
    void drawComingSoon();

    void handleListInput(InputEvent event);
    void handleRawListeningInput(InputEvent event);
    void handleComingSoonInput(InputEvent event);

    void startRawCapture();
    void stopRawCapture();
    void saveRawCaptureToSd();
};
