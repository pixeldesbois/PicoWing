#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <esp_now.h>
#include <WiFi.h>

// -------------------------------------------------
// Types de messages
// -------------------------------------------------
enum MsgType : uint8_t {
    MSG_PAIR_REQ    = 1,
    MSG_PAIR_ACK    = 2,
    MSG_CTRL_PWM    = 3,
    MSG_UNPAIR      = 4,
    MSG_TRIM_SET    = 5,
    MSG_TRIM_COMMIT = 6,
    MSG_BAT_LOW     = 0xF0,
    MSG_BAT_CRIT    = 0xF1
};

#pragma pack(push,1)
struct CtrlMsg {
    uint8_t type;
    uint8_t l; // PWM gauche
    uint8_t r; // PWM droite
    uint16_t seq;
    uint8_t flags;
};

struct TrimMsg {
    uint8_t type; // MSG_TRIM_SET
    int8_t trim;
};
#pragma pack(pop)

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

    // Callback pour recevoir messages
    void setRecvCallback(void (*cb)(const uint8_t* mac, const uint8_t* data, int len));

private:
    Preferences prefs;
    uint8_t pairedMac[6];
    bool paired;

    static void onDataRecvStatic(const uint8_t* mac, const uint8_t* data, int len);
    static void onDataSentStatic(const uint8_t* mac, esp_now_send_status_t status);
    void registerCallbacks();

    // Singleton pour les callbacks statiques
    static RadioManager* instance;
};
