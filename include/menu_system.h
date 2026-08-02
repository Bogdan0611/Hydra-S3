#pragma once
#include "app_interface.h"
#include "display_manager.h"
#include "input_manager.h"

// maxim de iteme in meniu - fixat dinainte ca sa nu folosesc alocare
// dinamica (new/delete), care pe un microcontroler poate fragmenta memoria
#define MAX_MENU_ITEMS 10

// cate iteme incap pe ecran deodata (64px, 12px intre randuri) - peste
// atat, meniul face scroll
#define MAX_VISIBLE_ITEMS 5

// meniul principal: lista verticala, UP/DOWN ca sa navighezi, OK ca sa
// deschizi o aplicatie, BACK (butonul SET) ca sa iesi din ea

struct MenuItem {
    const char* label;
    App* app;
};

class MenuSystem {
public:
    // adauga un item nou - se cheama in setup(), pentru fiecare modul
    void addItem(const char* label, App* app);

    void begin(DisplayManager* disp);

    // se cheama mereu din loop() - decide daca deseneaza meniul sau
    // ruleaza aplicatia activa
    void update(InputEvent event);

private:
    MenuItem items[MAX_MENU_ITEMS];
    int itemCount = 0;
    int selectedIndex = 0;

    // primul item vizibil pe ecran - cand selectedIndex iese din
    // fereastra vizibila, mut scrollOffset ca sa il aduc inapoi
    int scrollOffset = 0;

    // nullptr = suntem in meniu. altfel, aplicatia asta ruleaza pe tot
    // ecranul si primeste tot input-ul
    App* activeApp = nullptr;

    DisplayManager* display = nullptr;

    void drawMenu();
    void handleMenuInput(InputEvent event);
    void adjustScroll();
};
