#pragma once
#include <U8g2lib.h>

// wrapper peste U8g2 pentru ecranul OLED - daca schimb vreodata ecranul
// sau libraria, schimb doar fisierul asta, restul codului nu stie ce
// e pe dedesubt

class DisplayManager {
public:
    // false daca ecranul nu a fost gasit
    bool begin();

    // sterge ecranul, se cheama la inceputul fiecarui desen
    void clear();

    // trimite ce am desenat catre ecran, se cheama la final -
    // fara asta nu se vede nimic
    void sendToScreen();

    // x, y = coltul stanga-jos al textului
    // inverted = deseneaza cu pixeli stinsi (pt itemul selectat din meniu,
    // care sta peste un dreptunghi plin)
    void drawText(int x, int y, const char* text, bool inverted = false);

    // dreptunghi plin, folosit pentru highlight-ul itemului selectat
    void drawSelectionBox(int x, int y, int width, int height);

    // sageti mici, pentru cand meniul are mai multe iteme decat incap pe ecran
    void drawArrowUp(int x, int y);
    void drawArrowDown(int x, int y);

private:
    // "F" = full buffer, tine tot ecranul in RAM inainte sa il trimita.
    // mai simplu decat modul "page" care economiseste RAM dar e mai
    // complicat de folosit
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2{U8G2_R0};
};
