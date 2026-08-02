#include "menu_system.h"
#include <Arduino.h>

void MenuSystem::addItem(const char* label, App* app) {
    if (itemCount >= MAX_MENU_ITEMS) {
        Serial.println("[EROARE] Meniul e plin! Mareste MAX_MENU_ITEMS din menu_system.h");
        return;
    }

    items[itemCount].label = label;
    items[itemCount].app = app;
    itemCount++;
}

void MenuSystem::begin(DisplayManager* disp) {
    display = disp;
}

void MenuSystem::update(InputEvent event) {
    if (activeApp != nullptr) {
        // suntem intr-o aplicatie (ex: Sub-GHz e deschis)

        if (event == InputEvent::BACK && activeApp->isAtTopLevel()) {
            // BACK din ecranul de baza al aplicatiei = iesim complet.
            // daca aplicatia e intr-un sub-ecran (isAtTopLevel = false),
            // nu interceptez BACK aici, il las sa treaca la onInput()
            // mai jos, ca aplicatia sa decida ea ce inseamna (de obicei
            // intoarcere la propriul ei meniu, nu iesire din tot)
            activeApp->onExit();
            activeApp = nullptr;
            return;
        }

        if (event != InputEvent::NONE) {
            activeApp->onInput(event);
        }
        activeApp->onLoop();
        return;
    }

    // suntem in meniul principal
    handleMenuInput(event);
    drawMenu();
}

void MenuSystem::handleMenuInput(InputEvent event) {
    if (itemCount == 0) {
        return;
    }

    switch (event) {
        case InputEvent::UP:
            // modulo ca sa sar la ultimul item cand sunt pe primul si
            // apas UP (meniu circular)
            selectedIndex = (selectedIndex - 1 + itemCount) % itemCount;
            adjustScroll();
            break;

        case InputEvent::DOWN:
            selectedIndex = (selectedIndex + 1) % itemCount;
            adjustScroll();
            break;

        case InputEvent::OK:
            activeApp = items[selectedIndex].app;
            activeApp->onEnter();
            break;

        default:
            // BACK, LEFT, RIGHT nu fac nimic in meniul principal
            break;
    }
}

void MenuSystem::adjustScroll() {
    if (selectedIndex < scrollOffset) {
        scrollOffset = selectedIndex;
    }

    if (selectedIndex >= scrollOffset + MAX_VISIBLE_ITEMS) {
        scrollOffset = selectedIndex - MAX_VISIBLE_ITEMS + 1;
    }
}

void MenuSystem::drawMenu() {
    display->clear();

    int lastVisibleIndex = min(scrollOffset + MAX_VISIBLE_ITEMS, itemCount) - 1;

    // "row" e pozitia pe ecran, "i" e pozitia reala in lista completa
    // (poate incepe de la un index mai mare, daca am facut scroll)
    int row = 0;
    for (int i = scrollOffset; i <= lastVisibleIndex; i++) {
        int yPosition = 12 + (row * 12);

        if (i == selectedIndex) {
            display->drawSelectionBox(0, yPosition - 9, 128, 11);
        }

        display->drawText(4, yPosition, items[i].label, i == selectedIndex);
        row++;
    }

    if (scrollOffset > 0) {
        display->drawArrowUp(122, 1);
    }

    if (lastVisibleIndex < itemCount - 1) {
        display->drawArrowDown(122, 58);
    }

    display->sendToScreen();
}
