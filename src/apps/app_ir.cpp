#include "apps/app_ir.h"
#include "config.h"
#include <Arduino.h>

static const char* IR_ITEM_NAMES[IR_ITEM_COUNT] = {
    "Receive Code",
    "Send Last Code",
    "Saved Codes",
    "Settings"
};

IrApp::IrApp(DisplayManager* disp)
    : display(disp),
      irrecv(PIN_IR_RX, IR_CAPTURE_BUFFER_SIZE, IR_TIMEOUT_MS, true),
      irsend(PIN_IR_TX) {}

void IrApp::onEnter() {
    // nu pornesc receptorul aici - il pornesc doar cand chiar intru pe
    // ecranul RECEIVE, altfel ramane sa capteze in fundal si degeaba
    irsend.begin();

    currentScreen = IrScreen::LIST;
    selectedIndex = 0;
}

void IrApp::onExit() {
    stopReceiver();
}

void IrApp::startReceiver() {
    if (!receiverEnabled) {
        irrecv.enableIRIn();
        receiverEnabled = true;
    }
}

void IrApp::stopReceiver() {
    if (receiverEnabled) {
        irrecv.disableIRIn();
        receiverEnabled = false;
    }
}

void IrApp::onInput(InputEvent event) {
    switch (currentScreen) {
        case IrScreen::LIST:
            handleListInput(event);
            break;
        case IrScreen::RECEIVE:
            handleReceiveInput(event);
            break;
        case IrScreen::COMING_SOON:
            handleComingSoonInput(event);
            break;
    }
}

void IrApp::onLoop() {
    switch (currentScreen) {
        case IrScreen::LIST:
            drawList();
            break;
        case IrScreen::RECEIVE:
            if (!codeReceived) {
                tryReceiveCode();
            }
            drawReceive();
            break;
        case IrScreen::COMING_SOON:
            drawComingSoon();
            break;
    }
}

// ------------------------------------------------------------
// ecranul LIST
// ------------------------------------------------------------

void IrApp::handleListInput(InputEvent event) {
    if (event == InputEvent::UP) {
        selectedIndex = (selectedIndex - 1 + IR_ITEM_COUNT) % IR_ITEM_COUNT;
    } else if (event == InputEvent::DOWN) {
        selectedIndex = (selectedIndex + 1) % IR_ITEM_COUNT;
    } else if (event == InputEvent::OK) {
        if (selectedIndex == 0) {
            codeReceived = false;
            startReceiver();
            currentScreen = IrScreen::RECEIVE;
        } else if (selectedIndex == 1) {
            sendLastCode();
        } else {
            currentScreen = IrScreen::COMING_SOON;
        }
    }
}

void IrApp::drawList() {
    display->clear();

    for (int i = 0; i < IR_ITEM_COUNT; i++) {
        int yPosition = 12 + (i * 12);
        bool isSelected = (i == selectedIndex);

        if (isSelected) {
            display->drawSelectionBox(0, yPosition - 9, 128, 11);
        }

        display->drawText(4, yPosition, IR_ITEM_NAMES[i], isSelected);
    }

    display->sendToScreen();
}

// ------------------------------------------------------------
// ecranul RECEIVE - citire live de la o telecomanda
// ------------------------------------------------------------

void IrApp::tryReceiveCode() {
    if (!irrecv.decode(&results)) {
        return;
    }

    if (results.value == kRepeat) {
        // multe telecomenzi (NEC si altele) trimit un semnal separat de
        // "tot butonul ala e apasat" cat tii degetul pe el, fara cod real -
        // value iese kRepeat (0xFFFF...F). il ignoram si ascultam mai departe
        irrecv.resume();
        return;
    }

    codeReceived = true;

    // opresc receptorul cat aratam rezultatul - altfel ramane sa capteze
    // in fundal si urmatoarea citire iese cu resturi vechi/gunoi
    stopReceiver();
}

void IrApp::drawReceive() {
    display->clear();

    if (!codeReceived) {
        display->drawText(4, 12, "Receive Code");
        display->drawText(4, 32, "Point a remote");
        display->drawText(4, 44, "and press a button");
        display->drawText(4, 60, "SET=back");
        display->sendToScreen();
        return;
    }

    display->drawText(4, 12, "Code received!");

    // typeToString si resultToHexidecimal sunt din IRutils.h - le
    // folosesc in loc de sprintf %llX pentru ca formatarea de uint64_t
    // nu merge sigur pe toate variantele de printf de pe ESP32
    String protocolName = typeToString(results.decode_type);
    char protocolText[32];
    snprintf(protocolText, sizeof(protocolText), "Protocol: %s", protocolName.c_str());
    display->drawText(4, 28, protocolText);

    String hexValue = resultToHexidecimal(&results);
    char valueText[32];
    snprintf(valueText, sizeof(valueText), "Value: %s", hexValue.c_str());
    display->drawText(4, 40, valueText);

    display->drawText(4, 60, "OK=again  SET=back");

    display->sendToScreen();
}

void IrApp::handleReceiveInput(InputEvent event) {
    if (event == InputEvent::OK && codeReceived) {
        // cautam un cod nou - repornesc receptorul, era oprit cat aratam rezultatul
        codeReceived = false;
        startReceiver();
    } else if (event == InputEvent::BACK) {
        stopReceiver();
        currentScreen = IrScreen::LIST;
    }
}

// ------------------------------------------------------------
// "Send Last Code" - retrimite ultimul cod primit
// ------------------------------------------------------------

void IrApp::sendLastCode() {
    display->clear();

    if (!codeReceived) {
        display->drawText(4, 24, "No code saved yet!");
        display->drawText(4, 40, "Receive one first");
        display->sendToScreen();
        delay(1200);
        return;
    }

    irsend.send(results.decode_type, results.value, results.bits);

    display->drawText(4, 24, "Code sent!");
    display->sendToScreen();
    delay(800);
}

// ------------------------------------------------------------
// ecranul COMING_SOON
// ------------------------------------------------------------

void IrApp::drawComingSoon() {
    display->clear();
    display->drawText(4, 20, IR_ITEM_NAMES[selectedIndex]);
    display->drawText(4, 36, "Coming soon...");
    display->drawText(4, 52, "Press any key");
    display->sendToScreen();
}

void IrApp::handleComingSoonInput(InputEvent event) {
    if (event != InputEvent::NONE) {
        currentScreen = IrScreen::LIST;
    }
}
