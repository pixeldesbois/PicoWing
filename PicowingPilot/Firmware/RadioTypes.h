// RadioTypes.h
#pragma once
#include <cstdint>

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
    uint8_t l;
    uint8_t r;
    uint16_t seq;
    uint8_t flags;
};

struct TrimMsg {
    uint8_t type;
    int8_t trim;
};

struct SimpleMsg {
    uint8_t type;
};
#pragma pack(pop)
