#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <esp_now.h>
#include <WiFi.h>
#include "RadioTypes.h"

// -------------------------------------------------
// RadioManager
// -------------------------------------------------
class RadioManager {
public:
    RadioManager();

    void begin();
    void setPairedMac(const uint8_t mac[6]);
    bool hasPaired() const;

    // Envoi
    esp_err_t sendCtrl(uint8_t l, uint8_t r, uint16_t seq=0, uint8_t flags=0);
    esp_err_t sendTrim(int8_t trim);
    esp_err_t sendPairReq();
    esp_err_t sendUnpair();
    esp_err_t sendSimple(uint8_t type); // pour MSG_BAT_LOW/CRIT etc.

    // Callback pour recevoir messages
    void setRecvCallback(void (*cb)(const uint8_t* mac, const uint8_t* data, int len));

private:
    Preferences prefs;
    uint8_t pairedMac[6];
    bool paired;

    static void onDataRecvStatic(const uint8_t* mac, const uint8_t* data, int len);
    static void onDataSentStatic(const uint8_t* mac, esp_now_send_status_t status);
    void (*recvCallback)(const uint8_t*, const uint8_t*, int) = nullptr;
    void registerCallbacks();

    // Singleton pour les callbacks statiques
    static RadioManager* instance;
};
