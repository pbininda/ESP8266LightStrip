#pragma once

#include "state.h"
#include "led.h"
#include "effects.h"

extern void initWiFi();
extern void handleWiFi(Led **led, Effects **effects);
extern void wiFiGoToSleep(uint32_t delayMs);
extern void startWiFiPortal();

extern String wiFiMac;
