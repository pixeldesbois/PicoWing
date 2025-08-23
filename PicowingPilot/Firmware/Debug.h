// ========================= Debug.h =========================
#pragma once
#include <Arduino.h>

// -----------------------------------------------------------
// Configuration compile-time
// DEBUG_LEVEL = 0 -> aucune trace compilée
// DEBUG_LEVEL = 1 -> traces normales
// DEBUG_LEVEL = 2 -> + verboses
// DEBUG_LEVEL >=3 -> + trace fine
// -----------------------------------------------------------
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 1
#endif


namespace dbg {


// État runtime (activable/désactivable sans recompiler)
extern bool enabled;


// Initialisation du port série pour le debug
void init(uint32_t baud = 115200, bool waitForPort = false);


// Activer/désactiver à chaud
void setEnabled(bool on);
bool isEnabled();


// Niveau de build (const)
constexpr int buildLevel() { return DEBUG_LEVEL; }


// Hexdump pratique
void hexdump(const void* data, size_t len, size_t cols = 16);


// Gestion d'assert (ne compile pas quand DEBUG_LEVEL=0)
void assertFailed(const char* expr, const char* file, int line);


} // namespace dbg


// -----------------------------------------------------------
// Macros de log (ne génèrent aucun code si DEBUG_LEVEL==0)
// -----------------------------------------------------------
#if DEBUG_LEVEL == 0
#define DBG(...) do{}while(0)
#define DBGV(...) do{}while(0)
#define DBGT(...) do{}while(0)
#define DBG_HEX(buf,len) do{}while(0)
#define DBG_ASSERT(x) do{}while(0)
#else
// INFO (niveau 1)
#define DBG(...) do{ if(::dbg::enabled && ::dbg::buildLevel()>=1) Serial.printf(__VA_ARGS__); }while(0)
// VERBOSE (niveau 2)
#define DBGV(...) do{ if(::dbg::enabled && ::dbg::buildLevel()>=2) Serial.printf(__VA_ARGS__); }while(0)
// TRACE (niveau 3+)
#define DBGT(...) do{ if(::dbg::enabled && ::dbg::buildLevel()>=3) Serial.printf(__VA_ARGS__); }while(0)
// HEXDUMP utilitaire
#define DBG_HEX(buf,len) do{ if(::dbg::enabled) ::dbg::hexdump((buf),(len)); }while(0)
// ASSERT
#define DBG_ASSERT(x) do{ if(!(x)) ::dbg::assertFailed(#x, __FILE__, __LINE__); }while(0)
#endif