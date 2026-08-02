#include "placeholder_app.h"

void PlaceholderApp::onEnter() {
    // nimic de facut, modulul asta nu are logica reala inca
}

void PlaceholderApp::onExit() {
}

void PlaceholderApp::onLoop() {
    display->clear();
    display->drawText(4, 20, appName);
    display->drawText(4, 35, "Coming soon...");
    display->drawText(4, 50, "Press SET to go back");
    display->sendToScreen();
}

void PlaceholderApp::onInput(InputEvent event) {
    // BACK e deja tratat de MenuSystem, nu am nevoie sa fac nimic aici
}
