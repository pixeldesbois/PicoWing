#include "RadioManager.h"
#include "Debug.h"

RadioManager* RadioManager::instance = nullptr;

RadioManager::RadioManager() : paired(false) {
    memset(pairedMac, 0, sizeof(pairedMac));
    instance = this;
}

void RadioManager::begin() {
    WiFi.mode(WIFI_STA);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);

    if (esp_now_init() != ESP_OK) {
        DBG("esp_now_init failed\n");
        ESP.restart();
    }

    registerCallbacks();

    // Restauration du MAC depuis NVS
    prefs.begin("picowing", true);
    size_t n = prefs.getBytes("mac", pairedMac, 6);
    if (n == 6) {
        paired = true;
        DBG("↩︎ MAC restaurée depuis NVS\n");

        // Ajout du peer connu
        esp_now_peer_info_t peer{};
        memcpy(peer.peer_addr, pairedMac, 6);
        peer.channel = 1;
        peer.encrypt = false;
        esp_now_add_peer(&peer);
    }
    prefs.end();
}

void RadioManager::registerCallbacks() {
    esp_now_register_recv_cb(onDataRecvStatic);
    esp_now_register_send_cb(onDataSentStatic);
}

void RadioManager::setRecvCallback(void (*cb)(const uint8_t*, const uint8_t*, int)) {
    recvCallback = cb;
}

bool RadioManager::hasPaired() const { return paired; }

void RadioManager::setPairedMac(const uint8_t mac[6]) {
    memcpy(pairedMac, mac, 6);
    paired = true;

    prefs.begin("picowing", false);
    prefs.putBytes("mac", mac, 6);
    prefs.end();

    esp_now_peer_info_t peer{};
    memcpy(peer.peer_addr, mac, 6);
    peer.channel = 1;
    peer.encrypt = false;
    esp_now_add_peer(&peer);
}

// --- Envoi ---
esp_err_t RadioManager::sendCtrl(uint8_t l, uint8_t r, uint16_t seq, uint8_t flags) {
    CtrlMsg msg{MSG_CTRL_PWM, l, r, seq, flags};
    return esp_now_send(pairedMac, (uint8_t*)&msg, sizeof(msg));
}

esp_err_t RadioManager::sendTrim(int8_t trim) {
    TrimMsg msg{MSG_TRIM_SET, trim};
    return esp_now_send(pairedMac, (uint8_t*)&msg, sizeof(msg));
}

esp_err_t RadioManager::sendPairReq() {
    uint8_t msg = MSG_PAIR_REQ;
    return esp_now_send((uint8_t*)"\xff\xff\xff\xff\xff\xff", &msg, 1); // broadcast
}

esp_err_t RadioManager::sendUnpair() {
    uint8_t msg = MSG_UNPAIR;
    return esp_now_send(pairedMac, &msg, 1);
}

// --- Callbacks statiques ---
void RadioManager::onDataRecvStatic(const uint8_t* mac, const uint8_t* data, int len) {
    if (instance && instance->recvCallback) {
        instance->recvCallback(mac, data, len);
    }
}

void RadioManager::onDataSentStatic(const uint8_t* mac, esp_now_send_status_t status) {
    (void)mac;
    (void)status;
}
