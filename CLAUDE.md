# Proiect: Flipper Zero clone pe ESP32-S3

## 0. Ce e acest fișier și de ce există

Acesta e fișierul `CLAUDE.md`. Claude Code îl citește **automat**, la începutul
fiecărei sesiuni noi, dacă e pus în rădăcina proiectului (`flipper-esp32/`).
Nu trebuie explicat nimic manual - conținutul de mai jos devine parte din
context de la prima interacțiune.

Notă onestă: documentația oficială Claude Code recomandă fișiere CLAUDE.md
sub ~200 de linii, ca instrucțiunile să nu se degradeze. Documentul ăsta e
mai lung, pentru că a fost cerut explicit "full full full". Dacă la un
moment dat observi că Claude Code ignoră sau confundă lucruri din el, cea
mai bună soluție e să-l tai în bucăți (`docs/hardware.md`, `docs/roadmap.md`
etc.) și să le legi din CLAUDE.md cu sintaxa `@docs/hardware.md`.

---

## 1. Viziunea proiectului

Dispozitiv hardware hacking tip Flipper Zero, construit de la zero pe
ESP32-S3, folosind module separate (nu un cip all-in-one ca la Flipper).
Scop dublu:
1. **Portofoliu de cybersecurity** - piesă demonstrativă pentru interviuri/
   internship-uri, în special orientate spre offensive security / red team.
2. **Acoperă tot ce face Flipper Zero, plus mai mult** - profitând de faptul
   că ESP32-S3 are WiFi + BLE native (Flipper nu are din fabrică), NRF24
   integrat (Flipper îl are doar ca modul extern opțional), și mult mai
   multă putere de procesare (dual-core 240MHz + 8MB PSRAM vs single-core
   64MHz al Flipper-ului).

Persoana din spatele proiectului (Bogdan) e student anul 2 la Computer
Science (Transilvania Brașov, cybersecurity focus), cu 4+ ani experiență
freelance IT. Preferă răspunsuri directe, fără umplutură/frază
motivațională, cod ultra-lizibil cu explicații "de ce", nu doar "ce".
Comunică în română - păstrează comentariile de cod și documentația în
română, ca restul proiectului.

---

## 2. Hardware complet - inventar și pinout exact

Toate componentele de mai jos sunt deja cumpărate și cablate fizic pe o
placă ESP32-S3 N16R8 (16MB Flash, 8MB PSRAM OPI) - varianta normală de
dev board, NU "Super Mini".

### 2.1 Alimentare
- Modul powerbank Li-ion (5V/2A, Type-C) → ESP32 5V/VIN
- Baterie Li-Ion 3.7V 2000mAh

### 2.2 Ecran OLED 0.96" (128x64, I2C)
| Pin ecran | Pin ESP32 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SCL | GPIO 9 |
| SDA | GPIO 8 |

Adresă I2C: `0x3C` (implicit pt. SSD1306; dacă rămâne negru, încearcă `0x3D`).

### 2.3 PN532 NFC/RFID (13.56MHz) - pe același bus I2C ca ecranul
| Pin PN532 | Pin ESP32 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SCL | GPIO 9 (comun cu ecranul) |
| SDA | GPIO 8 (comun cu ecranul) |
| IRQ | GPIO 38 |

Adresă I2C: `0x24`. **Important**: placa PN532 are 2 mini-switch-uri pentru
modul I2C/SPI/HSU - trebuie puse pe poziția I2C, altfel nu răspunde deloc,
indiferent cât de corect e cablat SDA/SCL. RSTPD_N (reset) nu e cablat -
opțional, majoritatea plăcilor au pull-up intern.

### 2.4 Bus SPI comun - CC1101, NRF24L01, MicroSD
| Semnal | Pin ESP32 |
|---|---|
| SCK | GPIO 12 |
| MISO | GPIO 13 |
| MOSI | GPIO 11 |

Fiecare modul de mai jos are propriul pin CS, dar toate 3 folosesc
SCK/MISO/MOSI de mai sus.

**CC1101 (Sub-1GHz, 315/433/868MHz)**
| Pin CC1101 | Pin ESP32 |
|---|---|
| VCC | 3V3 (**strict** - 5V îl arde) |
| GND | GND |
| CSN | GPIO 10 |
| GDO0 | GPIO 4 |
| GDO2 | GPIO 18 |

**NRF24L01+PA/LNA (2.4GHz)**
| Pin NRF24 | Pin ESP32 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| CSN | GPIO 14 |
| CE | GPIO 15 |
| IRQ | GPIO 16 |

**Notă importantă**: NRF24 NU are condensator de decuplare pus încă
(recomandat: 47-100µF electrolitic + 100nF ceramic pe VCC/GND, cât mai
aproape de modul). Vârfurile de curent la transmisie pot destabiliza tot
bus-ul SPI (inclusiv CC1101 și SD-ul) fără el. E un TODO hardware, nu
software.

**MicroSD**
| Pin SD | Pin ESP32 |
|---|---|
| CS | GPIO 21 |
| VCC | 3V3 sau 5V (dacă modulul are regulator onboard) |

### 2.5 Infraroșu & Buzzer
| Componentă | Pin ESP32 |
|---|---|
| IR TX (Transmitter) | GPIO 41 |
| IR RX (Receiver) | GPIO 40 |
| Buzzer | GPIO 42 |

### 2.6 Joystick 5 direcții + buton SET
| Buton | Pin ESP32 |
|---|---|
| UP | GPIO 1 |
| DOWN | GPIO 2 |
| LEFT | GPIO 5 |
| RIGHT | GPIO 6 |
| OK (centru) | GPIO 7 |
| SET (BACK) | GPIO 17 |
| COM | GND |

Toate configurate `INPUT_PULLUP` - apăsat = LOW.

### 2.7 Hardware care LIPSEȘTE / e amânat (și de ce)

- **RFID 125kHz (LF) scriere/clonare** - singurul modul găsit în România e
  RDM6300, care e **doar citire** (nu poate scrie/emula). Pentru
  citire+scriere ar trebui un modul bazat pe EM4095, negăsit local -
  necesită comandă AliExpress. Amânat.
- **iButton / 1-Wire probe** - NU e un modul de cumpărat, e literalmente
  2 fire de contact + rezistor pull-up ~4.7kΩ. Nu a fost construit încă,
  dar nu are sens să fie "căutat în magazin" - se face în 5 minute oricând.
- **BadUSB (HID prin USB nativ)** - ESP32-S3 are USB nativ (TinyUSB) care ar
  permite asta, dar pinii D+/D- nu sunt încă cablați pe placă. Amânat până
  se termină cablarea fizică.
- **Emulare NFC completă (fără card fizic, ca la Flipper)** - PN532 are
  teoretic `TgInitAsTarget`, dar e nesigur/limitat pentru Mifare Classic
  (anticoliziune + crypto1 nu sunt bine suportate). Fluxul realist cu PN532
  e citire → scriere pe "magic card" (UID rescriptibil), NU emulare live.
  Cardurile "magic" (blue fob + card alb din kit) au fost deja testate cu
  succes manual (citire + clonare fizică funcționează).

---

## 3. Arhitectura software

### 3.1 Pattern-ul central: `App` interface

Fiecare "aplicație" din meniu (Sub-GHz, NFC, IR, WiFi, BLE, NRF24)
moștenește clasa abstractă `App` (`include/app_interface.h`), cu 4 metode:

```cpp
virtual void onEnter() = 0;   // la deschiderea aplicației din meniu
virtual void onExit() = 0;    // la ieșire (BACK din meniul principal)
virtual void onLoop() = 0;    // apelat continuu cât aplicația e activă
virtual void onInput(InputEvent event) = 0; // input de la joystick
virtual const char* getName() = 0;
```

Meniul principal (`MenuSystem`) nu știe NIMIC despre ce face fiecare
aplicație intern - doar o pornește/oprește și îi trimite input. Fiecare
`App` poate avea propriul ei mini-meniu intern (vezi `SubGhzApp` - are
propriul `enum class SubGhzScreen` cu LIST / RAW_LISTENING / COMING_SOON).
Acesta e pattern-ul de urmat pentru orice modul nou.

### 3.2 Componentele "de bază" (infrastructură, nu se schimbă des)

- **`BusManager`** (`bus_manager.h/.cpp`) - inițializează I2C
  (`Wire.begin(sda,scl)`) și SPI (`SPI.begin(sck,miso,mosi,-1)`) pe pinii
  reali din `config.h`. Apelat primul lucru în `setup()`, înainte de orice
  modul care folosește aceste bus-uri.
- **`DisplayManager`** (`display_manager.h/.cpp`) - wrapper peste U8g2
  (`U8G2_SSD1306_128X64_NONAME_F_HW_I2C`). Metodă cheie: `drawText(x, y,
  text, inverted)` - parametrul `inverted` (implicit `false`) desenează
  textul cu pixeli stinși, folosit pentru itemul selectat din meniu (vezi
  4.2 - bug rezolvat).
- **`InputManager`** (`input_manager.h/.cpp`) - citește joystick-ul cu
  debounce (`INPUT_DEBOUNCE_MS` = 150ms), întoarce `enum class InputEvent`
  (UP/DOWN/LEFT/RIGHT/OK/BACK/NONE).
- **`StorageManager`** (`storage_manager.h/.cpp`) - inițializează SD-ul.
  SD e opțional la boot - dacă lipsește, meniul funcționează în continuare,
  doar modulele care chiar scriu pe SD verifică `isReady()` înainte.
- **`MenuSystem`** (`menu_system.h/.cpp`) - lista principală de aplicații,
  cu scroll automat (`MAX_VISIBLE_ITEMS = 5`, fereastră glisantă cu săgeți
  ▲▼ când sunt mai multe iteme decât încap pe cei 64px înălțime ecran).

### 3.3 Convenții de cod (de urmat pentru orice fișier nou)

- Comentarii **în română**, explică "de ce", nu doar "ce" face codul.
- Nume descriptive, fără abrevieri criptice (`selectedIndex`, nu `selIdx`).
- O funcție = o responsabilitate. Funcții peste ~20 linii se despart.
- DRY - fără logică duplicată.
- Error handling pe orice operație riscantă (SPI, SD, rețea) - mesaje
  clare ("[EROARE] ...") pe Serial, nu eșec silențios.
- Simplitate înainte de over-engineering. Fără dependințe externe
  inutile - dar librării hardware consacrate (U8g2, SmartRC-CC1101) sunt
  binevenite quando sunt necesare, nu de evitat cu orice preț.
- Evită: ternari imbricați, "magie" implicită, micro-optimizări premature.

---

## 4. Stadiul actual (ce e implementat vs. placeholder)

### 4.1 Funcțional
- Meniul principal complet (navigare, scroll, highlight corect)
- Bus I2C + SPI inițializate pe pinii reali
- **Sub-GHz → Citire RAW** - complet funcțional (vezi secțiunea 5)

### 4.2 Bug rezolvat - highlight de meniu "garbage"

Când itemul selectat era desenat, textul apărea corupt/ilizibil. Cauză:
`drawSelectionBox` desenează un dreptunghi plin cu culoarea implicită (1 =
aprins), iar `drawText` desena TOT cu culoarea 1 peste el - nicio diferență
de contrast. Fix: `drawText` are acum un parametru `inverted` care setează
`u8g2.setDrawColor(0)` înainte de a desena textul (apoi resetează la 1).
`MenuSystem::drawMenu()` și `SubGhzApp::drawList()` pasează
`i == selectedIndex` ca `inverted`. **Orice ecran nou cu listă selectabilă
trebuie să folosească același pattern** - altfel bug-ul reapare acolo.

### 4.3 Placeholder ("În curând")
- Sub-GHz: Citire decodată, Analizor de frecvențe, Semnale salvate, Setări
- NFC/RFID (PN532) - complet placeholder
- Infraroșu - complet placeholder
- WiFi Attacks - complet placeholder
- BLE Attacks - complet placeholder
- NRF24 (MouseJack) - complet placeholder

---

## 5. Detalii tehnice - Sub-GHz / Citire RAW (implementat)

### 5.1 Librăria CC1101

**`SmartRC-CC1101-Driver-Lib`** (LSatan), versiunea **V3.0.2** (reboot
2026). Atenție: versiunile vechi (până la 2.5.7) foloseau un obiect global
`ELECHOUSE_cc1101` cu variabile mirror în RAM. V3.0.0+ e complet
reproiectat, orientat pe obiecte (`SmartRC_CC1101 radio;`, poți avea
mai multe instanțe pentru module multiple). Wrapper de compatibilitate cu
codul vechi există, dar codul din acest proiect (`cc1101_radio.cpp`)
folosește direct API-ul nou.

Include: `#include "SmartRC_CC1101.h"` (nu vechiul
`<ELECHOUSE_CC1101_SRC_DRV.h>`).

`lib_deps` în `platformio.ini`:
```
https://github.com/LSatan/SmartRC-CC1101-Driver-Lib
```

### 5.2 Secvența de configurare (în `Cc1101Radio::startListening()`)

```cpp
chip.setSpiPin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_CC1101_CSN);
chip.setGDO(PIN_CC1101_GDO0, PIN_CC1101_GDO2);
chip.Init();
chip.setModulation(2);              // 2 = ASK/OOK
chip.setMHZ(frequencyMHz);
chip.setPktFormat(3);               // 3 = mod serial asincron (bypass FIFO)
chip.SpiWriteReg(CC1101_IOCFG0, 0x0D); // GDO0 = Serial Data Output brut
chip.SetRx();
```

De ce așa: modul normal (FIFO/pachete) al CC1101 așteaptă un protocol
cunoscut (sync word, CRC etc). Pentru RAW capture avem nevoie de "mod
serial asincron" - GDO0 devine o oglindă live a semnalului demodulat,
exact ca pinul DATA al unui modul RX ieftin de 433MHz. Ambele setări
(`setPktFormat(3)` ȘI `IOCFG0=0x0D`) sunt necesare simultan - una fără
cealaltă nu funcționează complet (confirmat din datasheet TI + surse
comunitare, nu doar presupus).

Frecvența implicită: `SUBGHZ_DEFAULT_MHZ = 433.92` (cea mai comună pentru
telecomenzi ieftine în Europa). Va deveni configurabilă când se
implementează "Analizor de frecvențe" / "Setări".

### 5.3 Capturarea propriu-zisă (`RawCapture`, `raw_capture.h/.cpp`)

Un interrupt `CHANGE` pe pinul GDO0 (`attachInterrupt`), care la fiecare
schimbare de nivel notează în `micros()` cât timp a stat pinul în starea
anterioară. Șirul acestor durate = semnalul RAW.

Detalii de reținut:
- ISR-ul trebuie marcat `IRAM_ATTR` (obligatoriu pe ESP32 pt. funcții
  apelate din interrupt).
- `attachInterrupt` cere o funcție liberă, nu o metodă de clasă - de-asta
  există `onEdgeStatic()` (static) + `activeInstance` (pointer static către
  instanța activă), care redirecționează către `onEdge()` (metoda reală).
- Bufferul (`RAW_CAPTURE_BUFFER_SIZE = 2000`) e `volatile unsigned long[]`
  - "volatile" pentru că se modifică din ISR, în afara fluxului normal.
- Când bufferul se umple, ISR-ul ignoră pulsuri noi (nu scrie în afara
  array-ului) - `isFull()` semnalează asta pe ecran.
- **Ordinea corectă la salvare**: `stopRawCapture()` ÎNAINTE de
  `saveRawCaptureToSd()` - nu invers. Altfel interruptul mai poate rula cât
  scriem pe SD (nu crapă, dar e sloppy și inutil de riscant).

### 5.4 Format fișier salvat (`/raw_0.txt`, `/raw_1.txt`, ...)

```
433920000          <- linia 1: frecvența ascultată, în Hz
1234               <- linia 2: durata primului puls, în microsecunde
567
890
...
```

TODO cunoscut, nerezolvat încă: fișierul nu reține polaritatea inițială
(pinul a pornit HIGH sau LOW?) - relevant pentru Replay/retransmitere,
care nu e implementată încă. De clarificat quando se implementează TX.

---

## 6. Roadmap - ce urmează (în ordine logică, cu observații)

1. **Sub-GHz → Analizor de frecvențe** - scanează benzile comune
   (315/433/868MHz) citind `radio.getSignalStrength()` (RSSI), ca să afli
   pe ce frecvență e telecomanda ta reală, fără să ghicești. Cel mai logic
   pas următor - rezolvă o problemă practică imediată (telecomenzile nu au
   fost încă testate cu succes pe 433.92MHz implicit).
2. **Sub-GHz → Semnale salvate** - listă din fișierele `/raw_*.txt` de pe
   SD (citite cu `SD.open` + parcurgere directory), navigare + retransmite
   (Replay = reface exact secvența de pulsuri prin GDO0 în TX). Rezolvă
   TODO-ul de polaritate de mai sus.
3. **Sub-GHz → Citire decodată** - recunoaștere automată de protocol
   (Came, Nice, Princeton). Mai complex - necesită o mică bibliotecă de
   pattern-uri cunoscute. Nu urgentă.
4. **NFC/PN532** - implementare reader Mifare Classic (citire UID prin
   I2C, adresă `0x24`). Bogdan a testat deja manual (cu alt software) că
   cardurile magic din kit funcționează pentru clonare - acum trebuie
   codul propriu-zis pe ESP32. Librărie recomandată: `Adafruit_PN532`.
   Emularea live NU e un obiectiv realist (vezi 2.7).
5. **Infraroșu (TX/RX)** - relativ simplu, librăria `IRremoteESP8266` sau
   echivalent acoperă majoritatea protocoalelor de telecomenzi TV/AC.
6. **WiFi Attacks** - deauth, evil twin/captive portal, PMKID capture.
   Atenție: pe ESP32, WiFi și BLE partajează același radio 2.4GHz - nu pot
   rula ambele simultan la capacitate maximă fără compromisuri. De discutat
   quando ajungem aici.
7. **BLE Attacks** - scanner, BLE spam (Apple/Samsung continuity),
   detector AirTag. Librărie: NimBLE-Arduino (mai eficientă ca BLE stack
   default Arduino-ESP32).
8. **NRF24 (MouseJack)** - librărie `RF24` (TMRh20). Necesită condensatorul
   de decuplare adăugat fizic înainte (vezi 2.4) - fără el, testele vor fi
   instabile și greu de diagnosticat.
9. **Buzzer feedback** - integrare simplă oricând (bip la apăsare, alertă
   la detecție card/semnal) - nu depinde de nimic altceva, poate fi făcut
   în paralel cu orice altceva.
10. **Amânate pe hardware, nu pe cod**: iButton (probe DIY needs building),
    RFID 125kHz scriere (EM4095 not sourced), BadUSB (D+/D- not wired).

---

## 7. Comenzi

```bash
cd flipper-esp32
pio run                    # compilează
pio run -t upload          # încarcă pe placă
pio device monitor         # Serial Monitor (Ctrl+C ca să ieși)
pio run -t upload -t monitor   # toate odată
```

Prima compilare descarcă automat librăriile din `lib_deps`
(U8g2 + SmartRC-CC1101-Driver-Lib) - durează puțin mai mult.

---

## 8. Context despre autor (opțional, dar util pentru ton și priorități)

Bogdan - student anul 2 Computer Science, Transilvania Brașov, focus pe
cybersecurity (offensive security / red team), 4+ ani experiență freelance
IT înainte de facultate. Acest proiect e o piesă de portofoliu pentru
internship-uri în cybersecurity, în paralel cu pregătire pentru certificări
(BTL1, apoi eJPT/PJPT). Preferă:
- Răspunsuri directe, fără "Great question!" sau umplutură motivațională
- Cod explicat pas cu pas, cu analogii, ca pentru cineva care încă învață
- Onestitate când ceva nu e sigur/testat, în loc de presupuneri prezentate
  ca fapte

Poți șterge sau edita această secțiune oricând - nu afectează codul.
