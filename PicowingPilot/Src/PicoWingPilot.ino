#include <Arduino.h>
#include <esp_sleep.h>
#include "RadioManager.h"
#include "RadioLedBridge.h"
#include "LedManager.h"
#include "RadioTypes.h"

#include "Debug.h"

// ==========================================================
// Brochage (Wemos LOLIN C3 Pico)
// ==========================================================
#define PIN_JOY_X   2    // ADC1_CH2
#define PIN_JOY_Y   3    // ADC1_CH3
#define PIN_BUTTON  4    // GPIO4 (réveil deep-sleep)
#define LED_PIN     7    // RGB LED WS2812

// ==========================================================
// Constantes
// ==========================================================
#define INACTIVITY_TIMEOUT 30000UL   // 30 sec avant deep sleep
#define ADC_MAX 4095                 // Résolution ADC ESP32-C3
#define DEADZONE 200                 // Zone morte joystick
#define CONFIG_WINDOW 30000UL        // 30s fenêtre config
#define JOY_THRESHOLD 800            // Zone pour le choix de l'appairage
// ==========================================================
// Instances globales
// ==========================================================
RadioManager radio;
LedManager leds(LED_PIN);
RadioLedBridge bridge(radio, leds);

// ==========================================================
// Etats et variables
// ==========================================================
static bool configMode = false;
static unsigned long configStart = 0;
static unsigned long lastActivity = 0;

static bool modeEasy = true; // coefficient de jeu
static int8_t trimValue = 0;

static unsigned long lastBtnPress = 0;
static bool btnWasDown = false;
static unsigned long btnDownTime = 0;
static const unsigned long LONG_PRESS_TIME = 300; // 300ms = appui long
static unsigned long lastTrimSent = 0;
const unsigned long TRIM_INTERVAL = 200; // ms (5 Hz max)

// ==========================================================
// Fonctions utilitaires
// ==========================================================
bool readJoystick(int16_t &jx, int16_t &jy) {
    int x = analogRead(PIN_JOY_X);
    int y = analogRead(PIN_JOY_Y);

    jx = x - (ADC_MAX / 2);
    jy = y - (ADC_MAX / 2);

    // Applique une zone morte
    if (abs(jx) < DEADZONE) jx = 0;
    if (abs(jy) < DEADZONE) jy = 0;

    return (jx != 0 || jy != 0);
}

bool readButton() {
    return digitalRead(PIN_BUTTON) == LOW; // actif bas
}

// ==========================================================
// Deep sleep si inactif
// ==========================================================
void checkInactivity() {
    if (millis() - lastActivity > INACTIVITY_TIMEOUT) {
        DBG("Inactivité détectée, passage en deep sleep");
        leds.setState(LedState::OFF);
        delay(100);

        esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BUTTON, 0); // wake par bouton
        esp_deep_sleep_start();
    }
}

// ==========================================================
// Radio callbacks
// ==========================================================
void onRadioReceive(const uint8_t* mac, const uint8_t* data, int len) {
    if (len <= 0) return;

    uint8_t type = data[0];

    switch (type) {
        case MSG_PAIR_ACK:
            leds.setPairAck();  // bleu fixe
            DBG("Pairing ACK reçu !");
            break;
        case MSG_BAT_LOW:
            leds.setBatteryLow();
            break;
        case MSG_BAT_CRIT:
            leds.setBatteryCrit();
            break;
        default:
            break;
    }
}

// ==========================================================
// Setup
// ==========================================================
void setup() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);

    DBG("=== PicoWingPilot démarrage ===");

    leds.begin();
    radio.begin();
 
    radio.addRecvHandler(onRadioReceive);
    bridge.begin();

    // Fenêtre config de 30s pour la pairing
    configStart = millis();
    configMode = true;
    leds.setPairingMode(); // clignotant bleu

    lastActivity = millis();
}

// ==========================================================
// Loop
// ==========================================================
void loop() {
    int16_t jx = 0, jy = 0;
    bool active = false;

    // Lecture joystick
    bool joyMoved = readJoystick(jx, jy);
    bool btn = readButton();

    // ======================================================
    // Gestion bouton (détection appui court / long)
    // ======================================================
    if (btn && !btnWasDown) {  
        // Nouveau press
        btnWasDown = true;
        btnDownTime = millis();
    }
    else if (!btn && btnWasDown) {  
        // Bouton relâché
        unsigned long pressDuration = millis() - btnDownTime;

        if (pressDuration < LONG_PRESS_TIME) {
            // Appui court → possible double clic
            if (millis() - lastBtnPress < 500) {
                modeEasy = !modeEasy;
                if (modeEasy) leds.setGameModeEasy();  
                else leds.setGameModeHard();
            }
            lastBtnPress = millis();
        }
        btnWasDown = false;
    }

    // ======================================================
    // Mode config : appui long + joystick haut/bas
    // ======================================================
    if (configMode && millis() - configStart < CONFIG_WINDOW) {
        if (btnWasDown && (millis() - btnDownTime > LONG_PRESS_TIME)) {
            if (jy > JOY_THRESHOLD) {
                DBG("Demande d'appairage (appui long)");
                radio.sendPairReq();
            } else if (jy < -JOY_THRESHOLD) {
                DBG("Demande de desappairage (appui long)");
                radio.sendUnpair();
            }
        }
    } else {
        configMode = false; 
        if (radio.hasPaired()) leds.setGameModeEasy(); 
    }

    // ======================================================
    // Pilotage
    // ======================================================
    if (joyMoved && radio.hasPaired()) {
        uint8_t motorL = 0, motorR = 0;
        float coeff = modeEasy ? 1.25f : 1.0f;
        int16_t roll = jx / coeff;
        int16_t gas  = jy;

        motorL = constrain(gas - roll, 0, 255);
        motorR = constrain(gas + roll, 0, 255);

        bridge.sendMotorPWM(motorL, motorR);
        active = true;
    }

    // ======================================================
    // Trim : bouton maintenu + joystick gauche/droite
    // ======================================================
    if (btnWasDown && joyMoved) {
      if (millis() - lastTrimSent > TRIM_INTERVAL) {
        trimValue = map(jx, 
                        (jx > 0 ? 0 : -(ADC_MAX / 2)), 
                        (jx > 0 ? (ADC_MAX / 2) : 0), 
                        (jx > 0 ? 0 : -45), 
                        (jx > 0 ? 45 : 0));
        trimValue = constrain(trimValue, -45, 45);
        bridge.sendTrim(trimValue);
        active = true;
        lastTrimSent = millis(); // reset timer
      }
    }

    // ======================================================
    // Mise à jour LED / radio / inactivité
    // ======================================================
    bridge.update();
    leds.update();
    if (active) lastActivity = millis();
    checkInactivity();
}
