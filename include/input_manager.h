#pragma once

// tip strict pentru evenimente, ca sa nu le confund din greseala cu
// alte numere intregi prin cod - trebuie scris mereu InputEvent::UP
enum class InputEvent {
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    OK,
    BACK
};

class InputManager {
public:
    // configureaza pinii joystick-ului, o singura data in setup()
    void begin();

    // verifica joystick-ul si intoarce ce s-a apasat (NONE daca nimic nou).
    // se cheama in fiecare iteratie din loop()
    InputEvent poll();

private:
    unsigned long lastChangeTime = 0;

    // tin minte daca butonul e inca apasat de la ultima citire. fara
    // asta, o apasare LUNGA ar da evenimente repetate la fiecare
    // INPUT_DEBOUNCE_MS - am avut exact bug-ul asta, tinand un buton
    // apasat mai mult "sarea" 2 pasi in meniu
    bool buttonHeld = false;

    // citeste toti pinii si intoarce evenimentul brut, fara debounce
    InputEvent readRawEvent();
};
