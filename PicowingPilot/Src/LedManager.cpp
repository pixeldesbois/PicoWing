#include "LedManager.h"

LedManager::LedManager(uint8_t pin, uint8_t numPixels)
: strip(numPixels, pin, NEO_GRB + NEO_KHZ800),
  currentState(LedState::OFF),
  overrideLed(LedState::OFF),
  overrideTimer(0)
{}

void LedManager::begin(){
    strip.begin();
    strip.show();
}

void LedManager::setState(LedState state){
    currentState = state;
    overrideLed = LedState::OFF; // reset override
    applyState(state);
}

void LedManager::overrideState(LedState state, uint32_t durationMs){
    overrideLed = state;
    overrideTimer = millis() + durationMs;
}

void LedManager::update(){
    LedState active = overrideLed != LedState::OFF ? overrideLed : currentState;
    if (overrideLed != LedState::OFF && millis() > overrideTimer) {
        overrideLed = LedState::OFF;
        active = currentState;
    }
    applyState(active);
}

void LedManager::applyState(LedState state){
    uint32_t color = 0;
    bool blink = false;

    switch(state){
        case LedState::GREEN: color = strip.Color(0,255,0); break;
        case LedState::RED:   color = strip.Color(255,0,0); break;
        case LedState::BLUE:  color = strip.Color(0,0,255); break;
        case LedState::WHITE: color = strip.Color(255,255,255); break;
        case LedState::BLINK_ORANGE:color = strip.Color(255,128,0); blink=true;break;
        case LedState::BLINK_GREEN: color = strip.Color(0,255,0); blink=true; break;
        case LedState::BLINK_RED:   color = strip.Color(255,0,0); blink=true; break;
        case LedState::BLINK_BLUE:  color = strip.Color(0,0,255); blink=true; break;
        case LedState::BLINK_WHITE: color = strip.Color(255,255,255); blink=true; break;
        default: color = 0; break;
    }

    if(blink && (millis()/500)%2==0) color = 0;

    strip.setPixelColor(0,color);
    strip.show();
}

// Codes couleur standardisés
void LedManager::setPairingMode()     { setState(LedState::BLINK_BLUE); }
void LedManager::setPairAck()         { setState(LedState::BLUE); }
void LedManager::setTrimMode()        { setState(LedState::BLINK_WHITE); }
void LedManager::setGameModeEasy()    { setState(LedState::GREEN); }
void LedManager::setGameModeHard()    { setState(LedState::RED); }
void LedManager::setBatteryLow()      { setState(LedState::BLINK_ORANGE); }
void LedManager::setBatteryCrit()     { setState(LedState::BLINK_RED); }

