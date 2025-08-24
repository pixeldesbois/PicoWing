#include <Arduino.h>

#define DEBUG_LEVEL 2 // (optionnel) avant d'inclure Debug.h
#include "Debug.h"

#include "LedManager.h"
#include "RadioManager.h"
#include "RadioLedBridge.h"
#include "RadioTypes.h"


// ==========================================================
// Brochage (Wemos LOLIN C3 Pico)
// ==========================================================
#define PIN_JOY_X   2    // ADC1_CH2 (AX)
#define PIN_JOY_Y   3    // ADC1_CH3 (AY)
#define PIN_BUTTON  4    // GPIO4, bouton (réveil deep-sleep)
#define LED_PIN     7    // RGB LED WS2812 style sur Wemos C3 Pico

// ==========================================================
// Configuration
// ==========================================================
#define INACTIVITY_TIMEOUT 30000   // 30s sans activité → deep sleep
#define JOY_THRESHOLD      50      // seuil minimum de variation joystick

// Mode de jeu : facile = 1.25, difficile = 1
float gameMode = 1.25;

RadioManager radio;
LedManager led(LED_PIN);
RadioLedBridge bridge(radio, led);

unsigned long lastActivity = 0;   // dernière activité détectée
int lastJoyX = 0, lastJoyY = 0;   // pour détecter le mouvement

int gaz = 0;
int roulis = 0;
int trim = 0;

// ----------------------------------------------------------
void goToSleep() {
    Debug::log("Inactivité détectée → Deep Sleep...");
    led.setState(LedState::OFF); // couper LED

    // Config wakeup : bouton ou reset
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BUTTON, 0);

    delay(200); // laisser le temps au message série de partir
    esp_deep_sleep_start();
}


// === STATE ===
enum Mode { MODE_FACILE, MODE_DIFFICILE };
Mode modeJeu = MODE_FACILE;

bool configMode = false;
bool paired = false;
uint8_t pairedMAC[6] = {0}; // MAC de l’avion appairé

int gaz = 0;
int roulis = 0;
int trim = 0;

// === TIMER SLEEP ===
unsigned long lastActivity = 0;
const unsigned long INACTIVITY_TIMEOUT = 60000; // 1 min




// === SETUP ===
void setup() {
  dbg::init(115200, true);
  DBG("=== PicoWingPilot ===\n level=%d\n", dbg::buildLevel());

  // Initialisation radio
  radio.begin();
  // Initialisation LED
  led.begin();
  // Lien radio <-> LED
  bridge.begin();

  // Configurer bouton
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // Référence initiale joystick
  lastJoyX = analogRead(PIN_JOY_X);
  lastJoyY = analogRead(PIN_JOY_Y);
  lastActivity = millis();


  //setLed(LED_CYAN); // boot calibration joystick
  //overrideLedForEvent(LED_BLINK_CYAN, 2000);
  
  DBG("Système prêt !");
}

// ----------------------------------------------------------
void loop() {
    // Lecture joystick
    int joyX = analogRead(PIN_JOY_X);
    int joyY = analogRead(PIN_JOY_Y);

    // Détection activité joystick
    if (abs(joyX - lastJoyX) > JOY_THRESHOLD || abs(joyY - lastJoyY) > JOY_THRESHOLD) {
        lastActivity = millis();
        lastJoyX = joyX;
        lastJoyY = joyY;
    }

    // Détection bouton
    if (digitalRead(PIN_BUTTON) == LOW) {
        lastActivity = millis();
    }

    // Transmission
    radio.sendJoystick(joyX, joyY);

    // Gestion radio + LED
    radio.loop();
    led.loop();

    // Vérif inactivité
    if (millis() - lastActivity > INACTIVITY_TIMEOUT) {
        goToSleep();
    }

    delay(50);
}
