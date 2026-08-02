#include "display_manager.h"
#include <Arduino.h>

bool DisplayManager::begin() {
    bool success = u8g2.begin();

    if (!success) {
        // nu am ecran ca sa afisez eroarea pe el, deci o trimit pe Serial
        Serial.println("[EROARE] Ecranul OLED nu a raspuns! Verifica firele SDA/SCL si adresa I2C.");
    }

    return success;
}

void DisplayManager::clear() {
    u8g2.clearBuffer();
}

void DisplayManager::sendToScreen() {
    u8g2.sendBuffer();
}

void DisplayManager::drawText(int x, int y, const char* text, bool inverted) {
    u8g2.setFont(u8g2_font_6x10_tf);

    // setDrawColor(0) stinge pixeli in loc sa ii aprinda - il folosesc
    // cand textul sta peste dreptunghiul plin al itemului selectat,
    // altfel nu se mai citeste nimic (bug pe care l-am avut la inceput)
    u8g2.setDrawColor(inverted ? 0 : 1);
    u8g2.drawStr(x, y, text);

    // resetez la normal, ca sa nu strice urmatoarele desene
    u8g2.setDrawColor(1);
}

void DisplayManager::drawSelectionBox(int x, int y, int width, int height) {
    u8g2.drawBox(x, y, width, height);
}

void DisplayManager::drawArrowUp(int x, int y) {
    // triunghi cu varful in sus: 2 colturi jos, 1 sus
    u8g2.drawTriangle(x, y + 4, x + 4, y + 4, x + 2, y);
}

void DisplayManager::drawArrowDown(int x, int y) {
    // acelasi triunghi, rasturnat
    u8g2.drawTriangle(x, y, x + 4, y, x + 2, y + 4);
}
