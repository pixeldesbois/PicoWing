#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// Pin et nombre de LED
#define LED_PIN   7
#define LED_COUNT 1

// États possibles de la LED
enum LedState {
    LED_OFF,
    LED_RED,
    LED_GREEN,
    LED_BLUE,
    LED_ORANGE,
    LED_WHITE,
    LED_CYAN,
    LED_VIOLET,
    LED_BLINK_RED,
    LED_BLINK_GREEN,
    LED_BLINK_BLUE,
    LED_BLINK_ORANGE,
    LED_BLINK_WHITE,
    LED_BLINK_CYAN
};

class LedManager {
public:
    LedManager();
    void begin();
    void set(LedState state);
    void override(LedState state, unsigned long durationMs = 2000);
    void update();

private:
    Adafruit_NeoPixel strip;
    LedState current;
    LedState overrideState;
    unsigned long overrideUntil;
    bool blinkOn;

    void apply(LedState state, bool blinkPhase);
};
