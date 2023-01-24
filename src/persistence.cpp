#include <stdint.h>
#include <EEPROM.h>
#include "state.h"
#include "persistence.h"

struct header {
  uint16_t magic;
  uint16_t version;
};

static struct header expectedHeader = {0x1ED5, 6};

static void printSettings() {
  for (uint8_t i = 0; i < NUM_STRIPS; i++) {
    Serial.print("Settings for strip "); Serial.println(i);
    Settings &settings = strip_settings[i];
    Serial.print("on: "); Serial.print(settings.on);
    Serial.print("    mode: "); Serial.println(settings.mode);
    Serial.print("r: "); Serial.print(settings.palette[settings.colidx].r);
    Serial.print("    g: "); Serial.print(settings.palette[settings.colidx].g);
    Serial.print("    b: "); Serial.println(settings.palette[settings.colidx].b);
    Serial.print("bri: "); Serial.println(settings.bri);
    Serial.print("bri2: "); Serial.println(settings.bri2);
    Serial.print("rise: "); Serial.print(settings.rise);
    Serial.print("    fall: "); Serial.println(settings.fall);
    Serial.print("cylcle: "); Serial.println(settings.cycle);
    Serial.print("onoffmode: "); Serial.println(settings.onoffmode);
  }
}

static void defaultSettings() {
  for (uint8_t i = 0; i < NUM_STRIPS; i++) {
    Settings &settings = strip_settings[i];
    settings.on = true;
    settings.mode = 0;
    settings.colidx = 0;
    settings.bri = 256;
    settings.bri2 = 10;
    settings.rise = 1000;
    settings.fall = 1000;
    settings.cycle = 10000;
    settings.onoffmode = 0;
    settings.ngradient = 1;
    settings.palette[0] = {255, 255, 255};
    settings.palette[1] = {255, 200, 120};
    settings.palette[2] = {255, 180, 100};
    settings.palette[3] = {255, 120, 50};
    settings.palette[4] = {255, 100, 40};
    settings.palette[5] = {255, 80, 30};
    settings.palette[6] = {255, 60, 20};
    settings.palette[7] = {255, 0, 0};
    settings.palette[8] = {255, 255, 255};
    settings.palette[9] = {255, 200, 120};
    settings.palette[10] = {255, 180, 100};
    settings.palette[11] = {255, 120, 50};
    settings.palette[12] = {255, 100, 40};
    settings.palette[13] = {255, 80, 30};
    settings.palette[14] = {255, 60, 20};
    settings.palette[15] = {255, 0, 0};
    settings.palette[16] = {255, 255, 255};
    settings.palette[17] = {255, 200, 120};
    settings.palette[18] = {255, 180, 100};
    settings.palette[19] = {255, 120, 50};
    settings.palette[20] = {255, 100, 40};
    settings.palette[21] = {255, 80, 30};
    settings.palette[22] = {255, 60, 20};
    settings.palette[23] = {255, 0, 0};
  }
}

void readSettings() {
  EEPROM.begin(512);
  struct header header;
  EEPROM.get(0, header);
  if (header.magic != expectedHeader.magic) {
    Serial.println("header magic mismatch");
    defaultSettings();
    printSettings();
    writeSettings();
    return;
  }
  else if (header.version != expectedHeader.version) {
    Serial.println("header version mismatch");
    defaultSettings();
    printSettings();
    writeSettings();
    return;
  }
  else {
    EEPROM.get(sizeof(struct header), strip_settings);
    for (uint8_t i = 0; i < NUM_STRIPS; i++) {
      Settings &settings = strip_settings[i];
      settings.on = true;
    }
    printSettings();
  }
}

void writeSettings() {
  EEPROM.put(0, expectedHeader);
  EEPROM.put(sizeof(struct header), strip_settings);
  EEPROM.commit();
}
