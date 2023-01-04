#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdint.h>

typedef struct StripSettings {
    // used in AP Title and HTTP title
    uint16_t NUM_LEDS;
    // lenght of connected LED strip
    uint8_t MAX_BRI2;
    // maximum allowed brightness leve (Max: 32)
    const char * STRIP_NAME;
    uint8_t LED_PIN;
    uint8_t CLOCK_PIN;
} StripSettings;

#define SYSTEM_NAME "Bettlicht 1"

static const uint8_t NUM_STRIPS = 2;
static constexpr StripSettings STRIP_SETTINGS[NUM_STRIPS] = {
    {
        10,
        20,
        "Bettlicht 1 Kopf",
        21, 19,
    },
    {
        10,
        20,
        "Bettlicht 1 Seite",
        18, 17
    },
    // {
    //     20,
    //     30,
    //     "Generisches Licht 3",
    //     16, 15
    // },
};

#endif