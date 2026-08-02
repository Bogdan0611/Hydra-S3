#pragma once
#include "app_interface.h"
#include "display_manager.h"

// ecran generic "Coming soon" pentru modulele pe care nu le-am
// implementat inca. cand incep sa fac unul de-adevaratelea, ii fac o
// clasa noua care mosteneste App (ca SubGhzApp/NfcApp) si o pun in
// main.cpp in loc de PlaceholderApp - restul (meniul) nu se schimba

class PlaceholderApp : public App {
public:
    PlaceholderApp(const char* name, DisplayManager* disp)
        : appName(name), display(disp) {}

    void onEnter() override;
    void onExit() override;
    void onLoop() override;
    void onInput(InputEvent event) override;
    const char* getName() override { return appName; }

private:
    const char* appName;
    DisplayManager* display;
};
