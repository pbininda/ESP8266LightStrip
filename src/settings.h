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

#ifdef BADOBEN
// IP 192.168.1.58
#define SYSTEM_NAME "Bad Oben"
static const uint8_t NUM_STRIPS = 1;
static constexpr StripSettings STRIP_SETTINGS[NUM_STRIPS] = {
    { 69, 20, "Bad Oben", 21, 19 },
};
#elif BETT1
// IP 192.168.1.136
#define SYSTEM_NAME "Bettlicht 1"
static const uint8_t NUM_STRIPS = 2;
static constexpr StripSettings STRIP_SETTINGS[NUM_STRIPS] = {
    { 60, 31, "Bettlicht 1 Kopf", 21, 19 },
    { 120, 16, "Bettlicht 1 Seite", 18, 17 },
};
#elif BETT2
// IP 192.168.1.137
#define SYSTEM_NAME "Bettlicht 2"
static const uint8_t NUM_STRIPS = 2;
static constexpr StripSettings STRIP_SETTINGS[NUM_STRIPS] = {
    { 60, 31, "Bettlicht 2 Kopf", 21, 19 },
    { 120, 16, "Bettlicht 2 Seite", 18, 17 },
};
#elif KUECHE1
// IP 192.168.1.56
#define SYSTEM_NAME "Kueche"
static const uint8_t NUM_STRIPS = 1;
static constexpr StripSettings STRIP_SETTINGS[NUM_STRIPS] = {
    { 105, 31, "Kueche 1", 21, 19 },
};
#elif ESPTEST
// IP 192.168.1.58
#define SYSTEM_NAME "Esp Test"
static const uint8_t NUM_STRIPS = 1;
static constexpr StripSettings STRIP_SETTINGS[NUM_STRIPS] = {
    { 69, 20, "Esp Test", 21, 19 },
};
#else
#error define a system config
#endif

#endif
