#pragma once
#include "app_interface.h"
#include "display_manager.h"
#include "storage_manager.h"
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

#define IR_ITEM_COUNT 4

// buffer-ul in care biblioteca aduna pulsurile brute cat asculta -
// 1024 e valoarea recomandata in exemplele IRremoteESP8266
#define IR_CAPTURE_BUFFER_SIZE 1024
#define IR_TIMEOUT_MS 15

// cate coduri salvate afisez in lista - fara scroll, tin simplu, incap
// exact 5 randuri pe ecranul de 64px
#define IR_MAX_SAVED_CODES 5

// aplicatia IR, acelasi tipar ca la Sub-GHz si NFC: meniu intern cu
// cateva functii, "Receive Code", "Send Last Code" si "Saved Codes"
// sunt functionale

enum class IrScreen {
    LIST,
    RECEIVE,
    SAVED_LIST,
    COMING_SOON
};

// un cod IR salvat pe SD - retinem doar ce are nevoie irsend.send() ca
// sa-l poata retrimite mai tarziu
struct SavedIrCode {
    int decodeType;
    uint64_t value;
    uint16_t bits;
};

class IrApp : public App {
public:
    IrApp(DisplayManager* disp, StorageManager* storageManager);

    void onEnter() override;
    void onExit() override;
    void onLoop() override;
    void onInput(InputEvent event) override;
    const char* getName() override { return "Infrared (IR)"; }

    bool isAtTopLevel() override { return currentScreen == IrScreen::LIST; }

private:
    DisplayManager* display;
    StorageManager* storage;
    IRrecv irrecv;
    IRsend irsend;

    IrScreen currentScreen = IrScreen::LIST;
    int selectedIndex = 0;

    // ultimul cod primit, ramane retinut cat timp aplicatia e deschisa,
    // ca sa poata fi retrimis cu "Send Last Code" sau salvat pe SD
    bool codeReceived = false;
    decode_results results;

    // tin minte daca receptorul e pornit chiar acum - irrecv.enableIRIn()/
    // disableIRIn() nu suporta sa fie apelate de 2 ori la rand fara una
    // sa alterneze cu cealalta (am avut crash exact din asta). startReceiver()/
    // stopReceiver() de mai jos verifica flag-ul asta inainte sa apeleze,
    // ca sa nu se mai intample
    bool receiverEnabled = false;

    SavedIrCode savedCodes[IR_MAX_SAVED_CODES];
    int savedCodeCount = 0;
    int savedListIndex = 0;

    void drawList();
    void drawReceive();
    void drawSavedList();
    void drawComingSoon();

    void handleListInput(InputEvent event);
    void handleReceiveInput(InputEvent event);
    void handleSavedListInput(InputEvent event);
    void handleComingSoonInput(InputEvent event);

    void tryReceiveCode();
    void sendLastCode();
    void saveCurrentCode();
    void loadSavedCodes();

    void startReceiver();
    void stopReceiver();
};
