#pragma once
#include "LedManager.h"
#include "RadioManager.h"

class RadioLedBridge {
public:
    RadioLedBridge(RadioManager& radio, LedManager& led);

    void begin();
    void loop(); // à appeler dans loop() pour update LED

    // événements déclenchés par la télécommande
    void onEnterConfigMode();   // appui long joystick
    void onExitConfigMode();
    void onGameModeChanged(bool easy); // true = facile, false = dur
    void onTrimMode();           // double clic ou appui joystick

    // événements déclenchés par RadioManager
    void onPairAckReceived();
    void onBatteryLow();
    void onBatteryCrit();

private:
    RadioManager& radio;
    LedManager& led;

    bool configModeActive;
    bool trimModeActive;
    bool gameModeEasy;
};
