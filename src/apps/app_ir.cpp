#include "apps/app_ir.h"
#include "config.h"
#include <Arduino.h>
#include <SD.h>

static const char* IR_ITEM_NAMES[IR_ITEM_COUNT] = {
    "Receive Code",
    "Send Last Code",
    "Saved Codes",
    "Settings"
};

IrApp::IrApp(DisplayManager* disp, StorageManager* storageManager)
    : display(disp),
      storage(storageManager),
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
        case IrScreen::SAVED_LIST:
            handleSavedListInput(event);
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
        case IrScreen::SAVED_LIST:
            drawSavedList();
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
        } else if (selectedIndex == 2) {
            loadSavedCodes();
            savedListIndex = 0;
            currentScreen = IrScreen::SAVED_LIST;
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

    display->drawText(4, 52, "OK=again   R=save");
    display->drawText(4, 60, "SET=back");

    display->sendToScreen();
}

void IrApp::handleReceiveInput(InputEvent event) {
    if (event == InputEvent::OK && codeReceived) {
        // cautam un cod nou - repornesc receptorul, era oprit cat aratam rezultatul
        codeReceived = false;
        startReceiver();
    } else if (event == InputEvent::RIGHT && codeReceived) {
        saveCurrentCode();
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
// salvarea codului curent pe SD (buton RIGHT din ecranul RECEIVE)
// ------------------------------------------------------------

void IrApp::saveCurrentCode() {
    display->clear();

    if (!storage->isReady()) {
        display->drawText(4, 24, "Eroare: SD lipsa!");
        display->sendToScreen();
        delay(1000);
        return;
    }

    // format simplu, o linie de text per cod: tip_protocol valoare_hex biti.
    // scriu cu FILE_APPEND ca sa adaug la finalul fisierului, nu sa il rescriu
    File file = SD.open("/ir_codes.txt", FILE_APPEND);
    if (!file) {
        display->drawText(4, 24, "Nu am putut salva");
        display->sendToScreen();
        delay(1000);
        return;
    }

    size_t sizeBefore = file.size();

    file.print((int)results.decode_type);
    file.print(' ');
    file.print(resultToHexidecimal(&results));
    file.print(' ');
    file.println(results.bits);
    file.close();

    // file.print() poate "reusi" fara sa arunce eroare chiar daca cardul
    // nu retine nimic fizic (card mort/write-protejat) - redeschid si
    // verific ca marimea a crescut cu adevarat, altfel "Salvat!" e minciuna
    File verify = SD.open("/ir_codes.txt");
    size_t sizeAfter = verify ? verify.size() : 0;
    verify.close();

    if (sizeAfter <= sizeBefore) {
        Serial.println("[EROARE] ir_codes.txt nu a crescut dupa scriere - cardul nu retine datele.");
        display->drawText(4, 24, "EROARE: cardul nu");
        display->drawText(4, 40, "retine scrierea!");
        display->sendToScreen();
        delay(1500);
        return;
    }

    display->drawText(4, 24, "Salvat!");
    display->sendToScreen();
    delay(800);
}

// ------------------------------------------------------------
// ecranul SAVED_LIST - codurile salvate pe SD
// ------------------------------------------------------------

void IrApp::loadSavedCodes() {
    savedCodeCount = 0;

    if (!storage->isReady()) {
        return;
    }

    File file = SD.open("/ir_codes.txt");
    if (!file) {
        // nu exista inca niciun cod salvat, e ok, lista ramane goala
        return;
    }

    while (file.available() && savedCodeCount < IR_MAX_SAVED_CODES) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) {
            continue;
        }

        int firstSpace = line.indexOf(' ');
        int secondSpace = line.indexOf(' ', firstSpace + 1);
        if (firstSpace == -1 || secondSpace == -1) {
            continue; // linie stricata, o sar
        }

        String typeStr = line.substring(0, firstSpace);
        String valueStr = line.substring(firstSpace + 1, secondSpace);
        String bitsStr = line.substring(secondSpace + 1);

        // strtoull citeste direct hex-ul cu "0x" din fata, fara sa mai trebuiasca
        // sa-l scot manual - mai sigur decat sscanf %llx pe ESP32
        savedCodes[savedCodeCount].decodeType = typeStr.toInt();
        savedCodes[savedCodeCount].value = strtoull(valueStr.c_str(), nullptr, 16);
        savedCodes[savedCodeCount].bits = bitsStr.toInt();
        savedCodeCount++;
    }

    file.close();
}

void IrApp::drawSavedList() {
    display->clear();

    if (savedCodeCount == 0) {
        display->drawText(4, 24, "No saved codes");
        display->drawText(4, 40, "SET=back");
        display->sendToScreen();
        return;
    }

    for (int i = 0; i < savedCodeCount; i++) {
        int yPosition = 12 + (i * 12);
        bool isSelected = (i == savedListIndex);

        if (isSelected) {
            display->drawSelectionBox(0, yPosition - 9, 128, 11);
        }

        String protocolName = typeToString((decode_type_t)savedCodes[i].decodeType);
        char lineText[24];
        snprintf(lineText, sizeof(lineText), "%d. %s", i + 1, protocolName.c_str());
        display->drawText(4, yPosition, lineText, isSelected);
    }

    display->sendToScreen();
}

void IrApp::handleSavedListInput(InputEvent event) {
    if (savedCodeCount == 0) {
        if (event == InputEvent::BACK) {
            currentScreen = IrScreen::LIST;
        }
        return;
    }

    if (event == InputEvent::UP) {
        savedListIndex = (savedListIndex - 1 + savedCodeCount) % savedCodeCount;
    } else if (event == InputEvent::DOWN) {
        savedListIndex = (savedListIndex + 1) % savedCodeCount;
    } else if (event == InputEvent::OK) {
        SavedIrCode code = savedCodes[savedListIndex];
        irsend.send((decode_type_t)code.decodeType, code.value, code.bits);

        display->clear();
        display->drawText(4, 24, "Code sent!");
        display->sendToScreen();
        delay(800);
    } else if (event == InputEvent::BACK) {
        currentScreen = IrScreen::LIST;
    }
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
