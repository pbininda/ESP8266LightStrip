#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdint.h>

typedef struct StripSettings {
    // used in AP Title and HTTP title
    uint16_t NUM_LEDS;
    // lenght of connected LED strip
    uint8_t MAX_BRI2;
    // maximum allowed brightness leve (Max: 32)
    const char * SYSTEM_NAME;
    uint8_t LED_PIN;
    uint8_t CLOCK_PIN;
} StripSettings;

static const uint8_t NUM_STRIPS = 3;

static constexpr StripSettings STRIP_SETTINGS[NUM_STRIPS] = {
    {
        10,
        10,
        "Generisches Licht 1",
        21, 19,
    },
    {
        5,
        20,
        "Generisches Licht 2",
        18, 17
    },
    {
        20,
        30,
        "Generisches Licht 3",
        16, 15
    },
};

#endif
