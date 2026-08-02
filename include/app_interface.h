#pragma once
#include "input_manager.h"

// interfata comuna pentru toate "aplicatiile" din meniu (Sub-GHz, NFC, etc)
// meniul principal nu stie ce face fiecare modul pe dinauntru, doar
// apeleaza functiile astea 4 - practic e un contract pe care il respecta
// oricine adauga un modul nou

class App {
public:
    // apelata o singura data cand deschizi aplicatia din meniu
    virtual void onEnter() = 0;

    // apelata o singura data cand iesi (BACK din meniul principal)
    virtual void onExit() = 0;

    // apelata mereu, cat timp aplicatia e deschisa - aici e logica principala
    virtual void onLoop() = 0;

    // apelata cand utilizatorul apasa ceva pe joystick
    virtual void onInput(InputEvent event) = 0;

    // numele care apare in meniu
    virtual const char* getName() = 0;

    // true = aplicatia e in ecranul ei "de baza" (meniul intern), deci
    // BACK trebuie sa inchida toata aplicatia si sa te scoata la meniul
    // principal. false = esti intr-un sub-ecran (ex: citire live) si BACK
    // trebuie interpretat de aplicatie insasi, nu de MenuSystem - de-asta
    // MenuSystem verifica asta inainte sa iasa direct din aplicatie.
    // implicit true, bun pentru aplicatiile fara sub-ecrane (PlaceholderApp)
    virtual bool isAtTopLevel() { return true; }

    // destructor virtual, altfel memoria clasei copil nu s-ar elibera
    // corect daca stergem vreodata prin pointer App*
    virtual ~App() {}
};
