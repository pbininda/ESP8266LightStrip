#ifndef _WIFI_H_
#define _WIFI_H_

#include "state.h"
#include "led.h"

extern void initWiFi();
extern void handleWiFi(Led **led);
extern void wiFiGoToSleep(uint32_t delayMs);
extern void startWiFiPortal();

#endif

