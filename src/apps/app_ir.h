#pragma once
#include "app_interface.h"
#include "display_manager.h"
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

#define IR_ITEM_COUNT 4

// buffer-ul in care biblioteca aduna pulsurile brute cat asculta -
// 1024 e valoarea recomandata in exemplele IRremoteESP8266
#define IR_CAPTURE_BUFFER_SIZE 1024
#define IR_TIMEOUT_MS 15

// aplicatia IR, acelasi tipar ca la Sub-GHz si NFC: meniu intern cu
// cateva functii, "Receive Code" si "Send Last Code" sunt functionale

enum class IrScreen {
    LIST,
    RECEIVE,
    COMING_SOON
};

class IrApp : public App {
public:
    explicit IrApp(DisplayManager* disp);

    void onEnter() override;
    void onExit() override;
    void onLoop() override;
    void onInput(InputEvent event) override;
    const char* getName() override { return "Infrared (IR)"; }

    bool isAtTopLevel() override { return currentScreen == IrScreen::LIST; }

private:
    DisplayManager* display;
    IRrecv irrecv;
    IRsend irsend;

    IrScreen currentScreen = IrScreen::LIST;
    int selectedIndex = 0;

    // ultimul cod primit, ramane retinut cat timp aplicatia e deschisa,
    // ca sa poata fi retrimis cu "Send Last Code"
    bool codeReceived = false;
    decode_results results;

    // tin minte daca receptorul e pornit chiar acum - irrecv.enableIRIn()/
    // disableIRIn() nu suporta sa fie apelate de 2 ori la rand fara una
    // sa alterneze cu cealalta (am avut crash exact din asta). startReceiver()/
    // stopReceiver() de mai jos verifica flag-ul asta inainte sa apeleze,
    // ca sa nu se mai intample
    bool receiverEnabled = false;

    void drawList();
    void drawReceive();
    void drawComingSoon();

    void handleListInput(InputEvent event);
    void handleReceiveInput(InputEvent event);
    void handleComingSoonInput(InputEvent event);

    void tryReceiveCode();
    void sendLastCode();

    void startReceiver();
    void stopReceiver();
};
