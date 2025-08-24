#pragma once
#include <Arduino.h>

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

// -------------------------------------------------
// Structures de messages
// -------------------------------------------------
#pragma pack(push,1)

// Contrôle moteur PWM
struct CtrlMsg {
    uint8_t type;     // MSG_CTRL_PWM
    uint8_t l;        // PWM gauche (0..255)
    uint8_t r;        // PWM droit (0..255)
    uint16_t seq;     // compteur / anti-replay
    uint8_t flags;    // futur: trims, modes, etc.
};

// Trim avion
struct TrimMsg {
    uint8_t type;     // MSG_TRIM_SET
    int8_t trim;      // [-20..20] ou [-45..45] selon implémentation
};

// Message générique pour batterie
struct BatMsg {
    uint8_t type;     // MSG_BAT_LOW ou MSG_BAT_CRIT
    uint8_t value;    // réservé pour futur usage
};

#pragma pack(pop)

// -------------------------------------------------
// Utilitaires
// -------------------------------------------------
static inline bool isBatteryMsg(uint8_t type) {
    return type == MSG_BAT_LOW || type == MSG_BAT_CRIT;
}
