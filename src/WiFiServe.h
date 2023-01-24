#ifndef _WIFI_H_
#define _WIFI_H_

#include "state.h"
#include "led.h"
#include "effects.h"

extern void initWiFi();
extern void handleWiFi(Led **led, Effects **effects);
extern void wiFiGoToSleep(uint32_t delayMs);
extern void startWiFiPortal();

#endif

