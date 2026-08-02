#include "raw_capture.h"

RawCapture* RawCapture::activeInstance = nullptr;

void RawCapture::begin(int gdo0Pin) {
    pin = gdo0Pin;
    pinMode(pin, INPUT);
    activeInstance = this;
}

void IRAM_ATTR RawCapture::onEdgeStatic() {
    // verific ca exista o instanta activa, sa nu cad pe un pointer gol
    // daca interrupt-ul ar porni cumva inainte de begin()
    if (activeInstance != nullptr) {
        activeInstance->onEdge();
    }
}

void IRAM_ATTR RawCapture::onEdge() {
    if (!capturing) {
        return;
    }

    unsigned long now = micros();

    // retin durata doar daca mai am loc in buffer, altfel as scrie
    // in afara array-ului
    if (sampleCount < RAW_CAPTURE_BUFFER_SIZE) {
        buffer[sampleCount] = now - lastEdgeMicros;
        sampleCount++;
    }

    lastEdgeMicros = now;
}

void RawCapture::start() {
    sampleCount = 0;
    lastEdgeMicros = micros();
    capturing = true;

    // CHANGE = interrupt-ul se declanseaza la orice schimbare de nivel,
    // am nevoie de ambele fronturi (cat a stat aprins SI cat a stat stins)
    attachInterrupt(digitalPinToInterrupt(pin), onEdgeStatic, CHANGE);
}

void RawCapture::stop() {
    detachInterrupt(digitalPinToInterrupt(pin));
    capturing = false;
}

int RawCapture::getSampleCount() {
    return sampleCount;
}

unsigned long RawCapture::getSample(int index) {
    return buffer[index];
}

bool RawCapture::isFull() {
    return sampleCount >= RAW_CAPTURE_BUFFER_SIZE;
}
