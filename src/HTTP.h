#pragma once

#include "state.h"
#include "LED.h"
#include "effects.h"

extern void initServer(Settings *settings, State *state, Led **led, Effects **effects);
extern void handleServer();
