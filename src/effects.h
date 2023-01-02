#ifndef _EFFECTS_H_
#define _EFFECTS_H_

#include "settings.h"
#include "state.h"

class Effects {
    public:
        Effects(uint8_t stripNo, const Settings &settings, const State &state, Led &led) :
            stripNo(stripNo),
            stripSettings(STRIP_SETTINGS[stripNo]),
            settings(settings),
            state(state),
            led(led)
        {
        }

        void setLedsFixed(uint32_t c);
        void setLedsZylon();
        void setLedsRainbowCycle();
    
    private:
        uint8_t stripNo;
        const StripSettings &stripSettings;
        const Settings &settings;
        const State &state;
        Led &led;
};

#endif
