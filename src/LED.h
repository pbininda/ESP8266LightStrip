#pragma once

#define FASTLED_USE_GLOBAL_BRIGHTNESS 1
#include <FastLED.h>


#include "settings.h"
#include "state.h"

struct InternalRgbw {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t white;
} __attribute__((packed)) __attribute__((aligned(4)));

class Led {
    public:
        Led(uint8_t stripNo, const State &state);
        ~Led();
        Led(const Led&) = delete;
        Led & operator=(const Led&) = delete;
    
        static uint32_t ledColor(const InternalRgbw &rgbwColor);
        void setLed(uint16_t n, const InternalRgbw &rgbwColor);
        void setLedc(uint16_t n, uint32_t c);
        uint32_t getLed(uint16_t n) const;
        InternalRgbw getLedc(uint16_t n) const;
        bool sendLeds();
        time_t getLastLedChangeDelta() const;

        template<uint8_t LED_PIN, uint8_t CLOCK_PIN>
        void initLeds() {
            Serial.println(String("FastLED ledPin: ") + String(LED_PIN) + String(" clockPin: ") + String(CLOCK_PIN));
            pinMode(LED_PIN, OUTPUT);
            pinMode(CLOCK_PIN, OUTPUT);
            digitalWrite(LED_PIN, HIGH);
            digitalWrite(CLOCK_PIN, HIGH);
            FastLED.addLeds<APA102, LED_PIN, CLOCK_PIN, BGR, DATA_RATE_MHZ(1)>(static_cast<CRGB *>(fastLeds), stripSettings.NUM_LEDS + 1);
            sendLeds(); // Initialize all pixels to 'off'
        }
        const StripSettings &stripSettings;

    private:
        uint8_t stripNo;
        const State &state;


        InternalRgbw *leds;
        CRGB *fastLeds;
        bool ledsChanged = false;
        time_t lastLedChange = 0;

};
