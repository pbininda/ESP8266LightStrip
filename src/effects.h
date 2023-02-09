#pragma once

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

        void setLedsFixed();
        void setLedsZylon();
        void setLedsRainbowCycle();

        Palette dynGradColor(uint16_t ledIdx) const;


    private:
        uint8_t stripNo;
        const StripSettings &stripSettings;
        const Settings &settings;
        const State &state;
        Led &led;
};
