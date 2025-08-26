#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Preferences.h>
#include <Adafruit_NeoPixel.h>

// ==========================================================
// DEBUG
// ==========================================================
#define DEBUG 1
#if DEBUG
  #define DBG(...)   Serial.printf(__VA_ARGS__)
#else
  #define DBG(...)   do{}while(0)
#endif

#if DEBUG >= 2
  #define DBG_VERBOSE(...) Serial.printf(__VA_ARGS__)
#else
  #define DBG_VERBOSE(...) do{}while(0)
#endif

// ==========================================================
// Radio / Moteurs / Sécurité / Batterie
// ==========================================================
#define ESPNOW_CHANNEL 1
#define L_MOTOR_PIN 4
#define R_MOTOR_PIN 5
#define L_CH 0
#define R_CH 1
#define PWM_FREQ 20000
#define PWM_RES 8

#define RX_TIMEOUT_MS 900
#define PAIRING_TIMEOUT_MS 30000
#define BATTERY_UPDATE_PERIOD 1000     // ms pour lecture batterie
#define LED_UPDATE_PERIOD     150      // ms pour mise à jour LED

#define BATTERY_PIN 3       // GPIO3 = VBAT
#define BATTERY_LOW 3300
#define BATTERY_CRIT 3100


// ==========================================================
// NVS
// ==========================================================
Preferences prefs;
uint8_t pairedMac[6] = {0};
int8_t trimValue = 0;
static bool receivedFromPaired = false;
static uint32_t lastBatteryMsgMs = 0;

static inline bool macIsZero(const uint8_t m[6]) { for (int i=0;i<6;i++) if (m[i]!=0) return false; return true; }
static inline void macClear(uint8_t m[6]) { for(int i=0;i<6;i++) m[i]=0; }
static inline int8_t clampTrim(int v){ if(v<-20) return -20; if(v>20) return 20; return (int8_t)v; }
static inline int clamp255(int v){ if(v<0) return 0; if(v>255) return 255; return v; }

void loadConfig(){
  prefs.begin("picowing", true);
  size_t n = prefs.getBytes("mac", pairedMac, sizeof(pairedMac));
  if(n==0) DBG("Aucune MAC sauvegardée.\n");
  trimValue = clampTrim(prefs.getInt("trim", 0));
  DBG("Trim chargé: %d\n", trimValue);
  prefs.end();
}

void saveMac(const uint8_t *mac){
  prefs.begin("picowing", false);
  prefs.putBytes("mac", mac, 6);
  prefs.end();
  memcpy(pairedMac, mac, 6);
  DBG("MAC sauvegardée.\n");
}

void saveTrim(){ prefs.begin("picowing", false); prefs.putInt("trim", trimValue); prefs.end(); DBG("💾 Trim sauvegardé = %d\n", trimValue); }
void clearNVS(){ prefs.begin("picowing", false); prefs.clear(); prefs.end(); DBG("NVS cleared\n"); }
void clearPairedMAC(){ macClear(pairedMac); clearNVS(); receivedFromPaired = false; DBG("🔁 Pairing effacé\n"); }

// ==========================================================
// Moteurs
// ==========================================================
static uint8_t lastL=0, lastR=0;
void motorsInit(){
  ledcSetup(L_CH, PWM_FREQ, PWM_RES);
  ledcSetup(R_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(L_MOTOR_PIN, L_CH);
  ledcAttachPin(R_MOTOR_PIN, R_CH);
  ledcWrite(L_CH, 0);
  ledcWrite(R_CH, 0);
  DBG("Motors initialized\n");
}

void motorsWrite(uint8_t l, uint8_t r){
  int lAdj = l;
  int rAdj = r;
  if (trimValue < 0) lAdj = clamp255(l + trimValue);
  else if (trimValue > 0) rAdj = clamp255(r + trimValue);
    // Ne mettre à jour que si différent de la dernière valeur
    if(lAdj != lastL){
        ledcWrite(L_CH, lAdj);
        lastL = lAdj;
    }
    if(rAdj != lastR){
        ledcWrite(R_CH, rAdj);
        lastR = rAdj;
    }
  DBG_VERBOSE("Motors PWM: L=%d R=%d\n", lAdj, rAdj);
}

// ==========================================================
// Protocole ESP-NOW
// ==========================================================
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
struct CtrlMsg { uint8_t type; uint8_t l; uint8_t r; uint16_t seq; uint8_t flags; };
struct TrimMsg { uint8_t type; int8_t trim; };
#pragma pack(pop)

static uint32_t lastRxMs = 0;
static uint32_t lastBatteryMs = 0;
static uint32_t lastLedUpdateMs = 0;
static uint16_t lastBatteryMv = 0;

void enterPairingNotice(){ DBG("⏳ Pas de MAC appairée -> attente d'appairage...\n"); }

static uint16_t lastSeq = 0;


bool seqIsNewer(uint16_t newSeq, uint16_t lastSeq){
    return (uint16_t)(newSeq - lastSeq) < 0x8000;
}

// ==== ESP-NOW callbacks ====
void onDataRecv(const uint8_t * mac, const uint8_t *data, int len){
  if(len < 1) return;
  uint8_t type = data[0];

  if (macIsZero(pairedMac)) {
    if (type == MSG_PAIR_REQ) {
      saveMac(mac);
      esp_now_peer_info_t peer{};
      memcpy(peer.peer_addr, mac, 6);
      peer.channel = ESPNOW_CHANNEL;
      peer.encrypt = false;
      esp_now_add_peer(&peer);
      uint8_t ack[1] = { MSG_PAIR_ACK };
      esp_now_send(mac, ack, sizeof(ack));
      DBG("✅ PAIR_REQ reçu. MAC liée\n");
    }
    return;
  }

  if (memcmp(mac, pairedMac, 6) != 0) return;
  receivedFromPaired = true;

  switch(type){
    case MSG_CTRL_PWM: {
    if(len < sizeof(CtrlMsg)) return;
    const CtrlMsg* m = (const CtrlMsg*)data;
	if(seqIsNewer(m->seq, lastSeq)){
		lastSeq = m->seq;
		motorsWrite(m->l, m->r);
		lastRxMs = millis();
	}
} break;

    case MSG_TRIM_SET: {
      if (len < (int)sizeof(TrimMsg)) return;
      const TrimMsg* t = (const TrimMsg*)data;
      trimValue = clampTrim(t->trim);
      DBG_VERBOSE("🎚️ Trim RAM = %d\n", trimValue);
    } break;

    case MSG_TRIM_COMMIT: saveTrim(); break;
    case MSG_UNPAIR: clearPairedMAC(); break;
    default: break;
  }
}
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status){ (void)mac_addr; (void)status; }

// ==========================================================
// Batterie
// ==========================================================
uint16_t readBatteryMilliVolts(){
  int raw = analogRead(BATTERY_PIN);
  float voltage = (raw / 4095.0) * 3.3 * 2; // diviseur 2:1
  return (uint16_t)(voltage * 1000);
}

void checkBattery(uint16_t batteryMv){
    if (batteryMv <= BATTERY_CRIT){
        motorsWrite(0,0);
        DBG("⚡ Batterie critique: %dmV -> moteurs arrêtés\n", batteryMv);
        uint8_t msg[2] = { MSG_BAT_CRIT, 0 };
        esp_now_send(pairedMac, msg, sizeof(msg));
    } 
    else if (batteryMv <= BATTERY_LOW){
        DBG("⚡ Batterie faible: %dmV\n", batteryMv);
        uint8_t msg[2] = { MSG_BAT_LOW, 0 };
        esp_now_send(pairedMac, msg, sizeof(msg));
    }
}

// ==========================================================
// LED RGB (WS2812B sur GPIO7) - optimisée
// ==========================================================
#define LED_PIN 7
#define LED_MAX_BRIGHTNESS 128
#define LED_BLINK_PERIOD 2000
static uint32_t lastLedUpdateMs = 0;
const uint32_t LED_UPDATE_PERIOD = 150; // ms
static uint32_t lastColor = 0xFFFFFFFF;
Adafruit_NeoPixel strip(1, LED_PIN, NEO_GRB + NEO_KHZ800);

void ledInit() {
  strip.begin();
  strip.show();
}

void ledSet(uint8_t r, uint8_t g, uint8_t b) {
  r = (r * LED_MAX_BRIGHTNESS) / 255;
  g = (g * LED_MAX_BRIGHTNESS) / 255;
  b = (b * LED_MAX_BRIGHTNESS) / 255;
  strip.setPixelColor(0, strip.Color(r,g,b));
  strip.show();
}

void ledUpdate(bool paired, uint16_t batteryMv, uint32_t ms) {
    bool blink = ((ms % LED_BLINK_PERIOD) < (LED_BLINK_PERIOD/2));
    uint8_t r=0, g=0, b=0;

    if(!paired) { r=0; g=0; b=blink ? LED_MAX_BRIGHTNESS : 0; }
    else if(batteryMv > BATTERY_LOW) { r=0; g=blink ? LED_MAX_BRIGHTNESS : 0; b=0; }
    else if(batteryMv > BATTERY_CRIT) { r=blink ? LED_MAX_BRIGHTNESS : 0; g=blink ? LED_MAX_BRIGHTNESS/2 : 0; b=0; }
    else { r=blink ? LED_MAX_BRIGHTNESS : 0; g=0; b=0; }

    uint32_t color = (r<<16)|(g<<8)|b;
    if(color != lastColor){
        lastColor = color;
        ledSet(r,g,b);
    }
}



// ==========================================================
// Setup / Loop
// ==========================================================
void setup(){
#if DEBUG
  Serial.begin(115200);
  delay(300);
  DBG("== PicoWing (avion) ==\n");
#endif
  motorsInit();
  ledInit();

  WiFi.mode(WIFI_STA);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK){
    DBG("❌ esp_now_init failed\n");
    while(1){ delay(1000); }
  }
  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  loadConfig();

  if(macIsZero(pairedMac)) enterPairingNotice();
  lastRxMs = millis();
}

void loop(){
  uint32_t now = millis();

  // Fail-safe
  if(!macIsZero(pairedMac) && now-lastRxMs>RX_TIMEOUT_MS) motorsWrite(0,0);

  // Auto-unpair si télécommande morte
  if(!macIsZero(pairedMac) && now-lastRxMs>PAIRING_TIMEOUT_MS) {
    DBG("⏳ Télécommande non répondante, unpair auto\n");
	motorsWrite(0,0);   // Stop moteurs
    clearPairedMAC();
  }

  // Batterie
	if(!macIsZero(pairedMac) && now - lastBatteryMs > BATTERY_UPDATE_PERIOD) {
		lastBatteryMv = readBatteryMilliVolts();
		checkBattery(lastBatteryMv);  // utiliser lastBatteryMv dans checkBattery si possible
		lastBatteryMs = now;
	}

    // ---- Mise à jour LED ----
    if(now - lastLedUpdateMs > LED_UPDATE_PERIOD) {
        ledUpdate(!macIsZero(pairedMac), lastBatteryMv, now);
        lastLedUpdateMs = now;
    }

}
