#include "LedManager.h"

LedManager::LedManager()
: strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800),
  current(LED_OFF),
  overrideState(LED_OFF),
  overrideUntil(0)
{}

void LedManager::begin() {
    strip.begin();
    strip.show();
}

void LedManager::set(LedState state) {
    current = state;
    overrideState = LED_OFF;
}

void LedManager::override(LedState state, unsigned long durationMs) {
    overrideState = state;
    overrideUntil = millis() + durationMs;
}

void LedManager::update() {
    LedState active = (overrideState != LED_OFF) ? overrideState : current;

    if (overrideState != LED_OFF && millis() > overrideUntil) {
        overrideState = LED_OFF;
        active = current;
    }

    bool blinkPhase = true;
    if (active >= LED_BLINK_RED && active <= LED_BLINK_CYAN) {
        blinkPhase = (millis() / 500) % 2 == 0;
    }

    apply(active, blinkPhase);
}

void LedManager::apply(LedState state, bool blinkPhase) {
    uint8_t r=0,g=0,b=0;

    switch(state) {
        case LED_RED:   case LED_BLINK_RED:   r=255; g=0;   b=0;   break;
        case LED_GREEN: case LED_BLINK_GREEN: r=0;   g=255; b=0;   break;
        case LED_BLUE:  case LED_BLINK_BLUE:  r=0;   g=0;   b=255; break;
        case LED_ORANGE:case LED_BLINK_ORANGE:r=255; g=128; b=0;   break;
        case LED_WHITE: case LED_BLINK_WHITE: r=255; g=255; b=255; break;
        case LED_CYAN:  case LED_BLINK_CYAN:  r=0;   g=255; b=255; break;
        case LED_VIOLET:                      r=255; g=0;   b=255; break;
        default: break; // LED_OFF
    }

    if (!blinkPhase) r=g=b=0;

    strip.setPixelColor(0, strip.Color(r,g,b));
    strip.show();
}

// --------------------------------------------------
// Fonctions standardisées pour la télécommande / avion
// --------------------------------------------------
void LedManager::setPairingMode()  { set(LED_BLINK_BLUE); }
void LedManager::setPairAck()      { set(LED_BLUE); }
void LedManager::setTrimMode()     { set(LED_BLINK_WHITE); }
void LedManager::setGameModeEasy() { set(LED_GREEN); }
void LedManager::setGameModeHard() { set(LED_RED); }
void LedManager::setBatteryLow()   { set(LED_BLINK_ORANGE); }
void LedManager::setBatteryCrit()  { set(LED_BLINK_RED); }
