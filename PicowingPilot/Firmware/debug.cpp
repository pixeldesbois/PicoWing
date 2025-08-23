// ========================= Debug.cpp =========================
#include "Debug.h"

namespace dbg {


bool enabled = (DEBUG_LEVEL>0);


void init(uint32_t baud, bool waitForPort){
#if DEBUG_LEVEL>0
Serial.begin(baud);
if(waitForPort){
unsigned long t0 = millis();
while(!Serial && (millis()-t0)<2000) { /* attendre max 2s */ }
}
enabled = true;
Serial.printf("\n[DBG] start (level=%d)\n", DEBUG_LEVEL);
#else
(void)baud; (void)waitForPort;
#endif
}


void setEnabled(bool on){ enabled = on; }
bool isEnabled(){ return enabled; }


void hexdump(const void* data, size_t len, size_t cols){
#if DEBUG_LEVEL>0
if(!enabled) return;
auto* p = static_cast<const uint8_t*>(data);
for(size_t i=0;i<len;i++){
if(i%cols==0){ Serial.printf("\n%04u : ", (unsigned)i); }
Serial.printf("%02X ", p[i]);
}
Serial.println();
#else
(void)data; (void)len; (void)cols;
#endif
}


void assertFailed(const char* expr, const char* file, int line){
#if DEBUG_LEVEL>0
Serial.printf("\n[ASSERT] %s @ %s:%d\n", expr, file, line);
// Sur MCU on évite abort(); on boucle afin de garder les logs visibles
for(;;){ delay(1000); }
#else
(void)expr; (void)file; (void)line;
#endif
}


} // namespace dbg

