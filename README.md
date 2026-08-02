# Flipper ESP32 - Schela OS / Meniu

Arhitectura de bază a "sistemului de operare" pentru proiectul tău. Meniul e
complet funcțional pe ecranul OLED. **Sub-GHz (Citire RAW)** e prima aplicație
reală, funcțională. Restul modulelor (NFC, IR, WiFi, BLE, NRF24) sunt încă
"placeholder"-e care afișează doar "În dezvoltare" - gata să fie înlocuite
cu logică reală, unul câte unul.

## Structura proiectului

```
flipper-esp32/
├── platformio.ini              # config PlatformIO + librării
├── include/
│   ├── config.h                # TOȚI pinii - deja setați cu cablarea ta reală
│   ├── app_interface.h         # "contractul" pe care îl respectă orice modul
│   ├── bus_manager.h           # inițializare I2C (OLED+PN532) și SPI (CC1101+NRF24+SD)
│   ├── input_manager.h         # citire joystick + debounce
│   ├── display_manager.h       # wrapper peste U8g2 (ecranul OLED)
│   ├── storage_manager.h       # init card SD
│   └── menu_system.h           # creierul meniului, cu scroll automat
├── src/
│   ├── main.cpp                # punctul de pornire, asamblează totul
│   ├── bus_manager.cpp
│   ├── input_manager.cpp
│   ├── display_manager.cpp
│   ├── storage_manager.cpp
│   ├── cc1101_radio.cpp        # configurare CC1101 (frecvență, mod ascultare)
│   ├── raw_capture.cpp         # cronometrare pulsuri RAW prin interrupt
│   ├── menu_system.cpp
│   └── apps/
│       ├── placeholder_app.cpp/.h   # ecranul "În dezvoltare" pt. module viitoare
│       └── app_subghz.cpp/.h        # Sub-GHz: Citire RAW (funcțional)
```

## Sub-GHz — Citire RAW (funcțional)

Prima funcție reală din proiect. Deschide **Sub-GHz (CC1101)** din meniu,
alege **Citire RAW**, apropie telecomanda de modul și apasă butonul ei.
Ecranul arată în timp real câte pulsuri au fost capturate și puterea
semnalului (RSSI). **OK** salvează captura pe SD, **SET** renunță.

Fișierele se salvează pe SD ca `/raw_0.txt`, `/raw_1.txt`, etc. Format:
prima linie = frecvența ascultată (în Hz), restul liniilor = durata
fiecărui puls (în microsecunde), una pe linie. Acest format e baza pentru
funcția de retransmitere (Replay) pe care o adăugăm ulterior.

Dacă ecranul arată "CC1101 negăsit!" la deschiderea aplicației: verifică
firele SPI (SCK/MISO/MOSI/CSN) și că modulul e alimentat strict la 3.3V,
niciodată 5V.

## Stare curentă

Pinii din `config.h` corespund deja cu cablarea ta reală (OLED, PN532 I2C,
bus SPI comun CC1101/NRF24/SD, IR, buzzer, joystick). Meniul face scroll
automat (max. 5 iteme vizibile simultan pe ecranul de 64px, cu săgeți sus/jos
când sunt mai multe) - poți adăuga oricâte module vrei fără sa se strice
layout-ul.

**Două lucruri de verificat fizic înainte de primul upload:**
1. **Switch-urile de mod de pe placa PN532** trebuie puse pe poziția **I2C**
   (majoritatea plăcilor PN532 au 2 mini-switch-uri pentru I2C/SPI/HSU).
2. **Condensator pe NRF24** (47-100µF electrolitic pe VCC/GND, cât mai
   aproape de modul) - fără el, vârfurile de curent la transmisie pot
   destabiliza tot bus-ul SPI (inclusiv CC1101 și SD-ul).

## Comenzi

```bash
# instalezi PlatformIO (o singură dată, dacă nu-l ai deja)
pip install platformio --break-system-packages

# te muți în folderul proiectului
cd flipper-esp32

# compilează
pio run

# încarcă pe placă (ESP32-S3 trebuie conectat prin USB)
pio run -t upload

# deschide Serial Monitor ca să vezi mesajele de debug (Ctrl+C ca să ieși)
pio device monitor

# sau toate trei odată
pio run -t upload -t monitor
```

## Cum extinzi cu module reale

Când vrei să implementezi, de exemplu, NFC/PN532 de-adevăratelea:

1. Creează `src/apps/app_nfc.h` și `.cpp`, cu o clasă `NfcApp : public App`
   (poți folosi `app_subghz.h/.cpp` ca model - are deja tiparul unei aplicații
   cu submeniu intern, ecran live și salvare pe SD)
2. Implementează logica reală în `onEnter()` (inițializare PN532),
   `onLoop()` (citire/desenare), `onInput()` (navigare în submeniul modulului)
3. În `main.cpp`, înlocuiește `PlaceholderApp appNfc(...)` cu
   `NfcApp appNfc(&displayManager, ...)` și adaugă orice alți parametri
   are nevoie clasa ta

Restul sistemului (meniu, input, ecran) nu se schimbă deloc.

## Probleme comune

**Ecranul rămâne negru:**
Verifică adresa I2C în `config.h` (`OLED_I2C_ADDRESS`) - majoritatea
modulelor SSD1306 folosesc `0x3C`, dar unele folosesc `0x3D`. Verifică și
firele SDA/SCL - dacă sunt inversate, ecranul nu răspunde deloc.

**"Failed to connect to ESP32" la upload:**
Unele plăci ESP32-S3 Super Mini nu intră automat în modul de programare.
Ține apăsat butonul BOOT de pe placă în timp ce rulează `pio run -t upload`,
și eliberează-l imediat ce vezi că a început să scrie ("Writing at...").

**PN532 nu răspunde (când îi adaugi driver-ul mai târziu):**
Aproape sigur switch-urile de pe placa PN532 nu sunt pe poziția I2C, sau
adresa I2C folosită în cod nu e `0x24`. Verifică serigrafia de pe modul.

**Cardul SD nu e găsit:**
Verifică pinul CS din `config.h` și că firele MOSI/MISO/SCK sunt pe pinii
hardware SPI corecți ai plăcii tale (nu orice pin GPIO merge la viteză mare
pentru SPI, deși tehnic poți face și SPI "software" pe orice pin, e mai lent).

**PSRAM nu e detectat / crash-uri random la boot:**
Verifică că `board_build.arduino.memory_type = qio_opi` din `platformio.ini`
corespunde exact cu modulul tău N16R8 (OPI PSRAM, nu QSPI).

**Meniul se mișcă de 2-3 ori la o singură apăsare:**
Mărește `INPUT_DEBOUNCE_MS` din `config.h` (încearcă 200-250ms).
