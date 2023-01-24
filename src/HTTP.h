#ifndef _HTTP_H_
#define _HTTP_H_

#include "state.h"
#include "led.h"
#include "effects.h"

extern void initServer(Settings *settings, State *state, Led **led, Effects **effects);
extern void handleServer();

#endif
