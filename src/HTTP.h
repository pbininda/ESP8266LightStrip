#ifndef _HTTP_H_
#define _HTTP_H_

#include "state.h"
#include "led.h"

extern void initServer(Settings *settings, State *state, Led **led);
extern void handleServer();

#endif
