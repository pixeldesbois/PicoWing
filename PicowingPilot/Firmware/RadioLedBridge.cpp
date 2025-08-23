#include "RadioLedBridge.h"

RadioLedBridge::RadioLedBridge(RadioManager& r, LedManager& l)
: radio(r), led(l),
  configModeActive(false),
  trimModeActive(false),
  gameModeEasy(true)
{}

void RadioLedBridge::begin() {
    // initialisation si besoin
}

void RadioLedBridge::loop() {
    // LED update obligatoire
    led.update();

    // Si mode config actif et pas de trim, clignotant bleu
    if(configModeActive && !trimModeActive) {
        led.setPairingMode();
    }

    // Si trim actif -> clignotant blanc
    if(trimModeActive) {
        led.setTrimMode();
    }

    // Si mode jeu
    if(!configModeActive && !trimModeActive) {
        if(gameModeEasy) led.setGameModeEasy();
        else             led.setGameModeHard();
    }
}

// --------------------------------------------------
// Méthodes pour la télécommande
// --------------------------------------------------
void RadioLedBridge::onEnterConfigMode() {
    configModeActive = true;
    trimModeActive = false;
}

void RadioLedBridge::onExitConfigMode() {
    configModeActive = false;
    trimModeActive = false;
}

void RadioLedBridge::onGameModeChanged(bool easy) {
    gameModeEasy = easy;
}

void RadioLedBridge::onTrimMode() {
    trimModeActive = true;
}

// --------------------------------------------------
// Méthodes appelées par les messages radio
// --------------------------------------------------
void RadioLedBridge::onPairAckReceived() {
    led.setPairAck();
}

void RadioLedBridge::onBatteryLow() {
    led.setBatteryLow();
}

void RadioLedBridge::onBatteryCrit() {
    led.setBatteryCrit();
}
