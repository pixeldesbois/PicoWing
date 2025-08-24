#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <esp_now.h>
#include <vector>
#include <functional>
#include <algorithm>
#include "RadioTypes.h"

// Type de callback pour réception
using RadioRecvHandler = std::function<void(const uint8_t* mac, const uint8_t* data, int len)>;

// Struct pour stocker un handler avec un ID unique
struct HandlerEntry {
    int id;
    RadioRecvHandler handler;
};

class RadioManager {
public:
    RadioManager();

    void begin(uint8_t ch = 1);
    void setPairedMac(const uint8_t mac[6]);
    bool hasPaired() const;

    // Envoi
    esp_err_t sendCtrl(uint8_t l, uint8_t r, uint16_t seq = 0, uint8_t flags = 0);
    esp_err_t sendTrim(int8_t trim);
    esp_err_t sendPairReq();
    esp_err_t sendUnpair();

    // Observers avec ID
    int addRecvHandler(RadioRecvHandler handler);   // retourne un ID unique
    void removeRecvHandler(int handlerId);          // supprime via l'ID

private:
    Preferences prefs;
    uint8_t pairedMac[6];
    bool paired;
    uint8_t channel;
    uint16_t seqCounter = 0; 

    std::vector<HandlerEntry> recvHandlers;
    int nextHandlerId = 1;   // compteur pour ID des handlers

    void handleRecv(const uint8_t* mac, const uint8_t* data, int len);

    // Callbacks statiques pour ESP-NOW
    static void onDataRecvStatic(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len);
    static void onDataSentStatic(const uint8_t* mac, esp_now_send_status_t status);

    // Singleton pour callback statique
    static RadioManager* instance;
};
