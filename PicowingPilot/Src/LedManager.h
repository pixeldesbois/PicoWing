#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

enum class LedState : uint8_t {
    OFF,
    GREEN,
    RED,
    BLUE,
    WHITE,
    BLINK_ORANGE,
    BLINK_GREEN,
    BLINK_RED,
    BLINK_BLUE,
    BLINK_WHITE,
};

class LedManager {
public:
    LedManager(uint8_t pin, uint8_t numPixels=1);

    void begin();
    void setState(LedState state);              // état courant
    void overrideState(LedState state, uint32_t durationMs); // override temporaire
    void update();                              // à appeler régulièrement

    // Codes couleur standardisés
    void setPairingMode();     // clignotant bleu
    void setPairAck();         // bleu fixe
    void setTrimMode();        // clignotant blanc
    void setGameModeEasy();    // vert fixe
    void setGameModeHard();    // rouge fixe
    void setBatteryLow();      // orange clignotant
    void setBatteryCrit();     // rouge clignotant

private:
    Adafruit_NeoPixel strip;
    LedState currentState;
    LedState overrideLed;
    uint32_t overrideTimer;

    void applyState(LedState state);
};
