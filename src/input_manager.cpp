#include "input_manager.h"
#include "config.h"
#include <Arduino.h>

void InputManager::begin() {
    // INPUT_PULLUP = pinul e HIGH cand butonul nu e apasat si LOW cand
    // face contact la GND. asa nu mai am nevoie de rezistente externe
    pinMode(PIN_JOY_UP, INPUT_PULLUP);
    pinMode(PIN_JOY_DOWN, INPUT_PULLUP);
    pinMode(PIN_JOY_LEFT, INPUT_PULLUP);
    pinMode(PIN_JOY_RIGHT, INPUT_PULLUP);
    pinMode(PIN_JOY_OK, INPUT_PULLUP);
    pinMode(PIN_BTN_SET, INPUT_PULLUP);
}

InputEvent InputManager::readRawEvent() {
    // prima apasare gasita castiga, nu ma astept sa apes 2 directii deodata
    if (digitalRead(PIN_JOY_UP) == LOW)    return InputEvent::UP;
    if (digitalRead(PIN_JOY_DOWN) == LOW)  return InputEvent::DOWN;
    if (digitalRead(PIN_JOY_LEFT) == LOW)  return InputEvent::LEFT;
    if (digitalRead(PIN_JOY_RIGHT) == LOW) return InputEvent::RIGHT;
    if (digitalRead(PIN_JOY_OK) == LOW)    return InputEvent::OK;
    if (digitalRead(PIN_BTN_SET) == LOW)   return InputEvent::BACK;

    return InputEvent::NONE;
}

InputEvent InputManager::poll() {
    InputEvent rawEvent = readRawEvent();

    if (rawEvent == InputEvent::NONE) {
        // nimic apasat, resetez - gata pentru o apasare noua
        buttonHeld = false;
        return InputEvent::NONE;
    }

    if (buttonHeld) {
        // e acelasi buton tinut de la citirea trecuta, il ignor
        return InputEvent::NONE;
    }

    // debounce: daca a trecut prea putin timp de la ultima apasare
    // valida, probabil e bounce mecanic, nu o apasare noua
    unsigned long now = millis();
    if (now - lastChangeTime < INPUT_DEBOUNCE_MS) {
        return InputEvent::NONE;
    }

    lastChangeTime = now;
    buttonHeld = true;
    return rawEvent;
}
