#pragma once

// toti pinii hardware intr-un singur loc, ca sa nu ii caut prin tot
// codul cand schimb ceva la cablaj. corespund cu placa mea reala,
// ESP32-S3 N16R8 (16MB flash, 8MB PSRAM)

// ------------------------------------------------------------
// ecran OLED (I2C) + PN532 NFC/RFID - pe acelasi bus I2C
// ------------------------------------------------------------
// I2C foloseste doar 2 fire pentru toate dispozitivele - merg pe acelasi
// SDA/SCL pentru ca fiecare are adresa lui (OLED = 0x3C, PN532 = 0x24)
#define PIN_I2C_SDA          8
#define PIN_I2C_SCL          9

#define OLED_WIDTH            128
#define OLED_HEIGHT           64
#define OLED_I2C_ADDRESS      0x3C   // daca ramane negru incearca 0x3D

#define PIN_PN532_IRQ         38
// RSTPD_N (reset) nu e cablat, majoritatea placilor au pull-up intern
// si merg oricum. Adafruit_PN532 vrea totusi un pin valid in constructor,
// asa ca dau unul liber, care nu e legat de nimic fizic.
#define PIN_PN532_RESET_NECONECTAT 3

// ------------------------------------------------------------
// bus SPI comun - CC1101, NRF24L01, microSD
// ------------------------------------------------------------
// SCK/MISO/MOSI sunt comune, dar fiecare modul are propriul pin CS
#define PIN_SPI_SCK           12
#define PIN_SPI_MISO          13
#define PIN_SPI_MOSI          11

// CC1101 (sub-1GHz)
#define PIN_CC1101_CSN        10
#define PIN_CC1101_GDO0       4
#define PIN_CC1101_GDO2       18

// NRF24L01+PA/LNA (2.4GHz)
#define PIN_NRF24_CSN         14
#define PIN_NRF24_CE          15
#define PIN_NRF24_IRQ         16

// microSD
#define PIN_SD_CS             21

// ------------------------------------------------------------
// infrarosu si buzzer
// ------------------------------------------------------------
#define PIN_IR_TX             41
#define PIN_IR_RX             40
#define PIN_BUZZER            42

// ------------------------------------------------------------
// joystick 5 directii + buton SET (=BACK)
// ------------------------------------------------------------
#define PIN_JOY_UP            1
#define PIN_JOY_DOWN          2
#define PIN_JOY_LEFT          5
#define PIN_JOY_RIGHT         6
#define PIN_JOY_OK            7
#define PIN_BTN_SET           17

// toti butonii sunt INPUT_PULLUP, deci apasat = LOW

// timp minim intre 2 apasari valide, ca sa ignor bounce-ul mecanic
// al butoanelor (contactul "sare" cateva ms cand apesi)
#define INPUT_DEBOUNCE_MS     150
