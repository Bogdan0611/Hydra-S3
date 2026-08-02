#pragma once
#include <Arduino.h>

// cate durate de puls pot retine intr-o captura. 2000 e destul pentru
// majoritatea telecomenzilor (o apasare = de obicei 20-200 pulsuri).
// cand se umple, capturarea continua dar ignora pulsurile noi (isFull())
#define RAW_CAPTURE_BUFFER_SIZE 2000

// masoara pulsurile semnalului radio brut de pe pinul GDO0 al CC1101
//
// cand CC1101 e in mod "serial asincron", GDO0 se aprinde/stinge exact
// cand semnalul RF receptionat se aprinde/stinge. ascult schimbarile
// astea cu un interrupt si notez cat timp a stat pinul in starea
// dinainte (in microsecunde) - sirul de durate e semnalul RAW, la fel
// cum salveaza si Flipper Zero la "Read RAW"

class RawCapture {
public:
    // configureaza pinul, o singura data inainte de primul start()
    void begin(int gdo0Pin);

    // goleste bufferul si incepe sa asculte
    void start();

    void stop();

    int getSampleCount();

    // durata in microsecunde a pulsului cu numarul index
    unsigned long getSample(int index);

    // true daca bufferul s-a umplut si au mai venit pulsuri neretinute
    bool isFull();

private:
    int pin = -1;

    // attachInterrupt vrea o functie libera, nu o metoda de clasa - de
    // asta am facut una statica + un pointer catre instanta activa
    static RawCapture* activeInstance;
    static void IRAM_ATTR onEdgeStatic();
    void IRAM_ATTR onEdge();

    // volatile pentru ca variabilele astea se schimba din interiorul
    // interrupt-ului (ISR), nu din fluxul normal al programului -
    // fara volatile compilatorul ar putea sa le optimizeze gresit
    volatile unsigned long buffer[RAW_CAPTURE_BUFFER_SIZE];
    volatile int sampleCount = 0;
    volatile unsigned long lastEdgeMicros = 0;
    volatile bool capturing = false;
};
