#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <esp_now.h>
#include <WiFi.h>
#include <vector>
#include <functional>
#include <algorithm>
#include "RadioTypes.h"

using RadioRecvHandler = std::function<void(const uint8_t* mac, const uint8_t* data, int len)>;

class RadioManager {
public:
    RadioManager();

    void begin(uint8_t ch = 1);
    void setPairedMac(const uint8_t mac[6]);
    bool hasPaired() const;

    // Envoi
    esp_err_t sendCtrl(uint8_t l, uint8_t r, uint16_t seq=0, uint8_t flags=0);
    esp_err_t sendTrim(int8_t trim);
    esp_err_t sendPairReq();
    esp_err_t sendUnpair();

    // Observers
    void addRecvHandler(RadioRecvHandler handler);
    void removeRecvHandler(RadioRecvHandler handler);

private:
    Preferences prefs;
    uint8_t pairedMac[6];
    bool paired;
    uint8_t channel;
    std::vector<RadioRecvHandler> recvHandlers;

    void handleRecv(const uint8_t* mac, const uint8_t* data, int len);
    //void (*recvCallback)(const uint8_t*, const uint8_t*, int) = nullptr;

    static void onDataRecvStatic(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len);
    static void onDataSentStatic(const uint8_t* mac, esp_now_send_status_t status);

    // Singleton pour callback statique
    static RadioManager* instance;
};
