#include <Arduino.h>
#include <EEPROM.h>
#include "state.h"
#include "persistence.h"

struct header {
  uint16_t magic;
  uint16_t version;
};

struct header expectedHeader = {0x1ED5, 2};

void printSettings() {
  Serial.print("on: "); Serial.print(settings.on);
  Serial.print("    mode: "); Serial.println(settings.mode);
  Serial.print("r: "); Serial.print(settings.r);
  Serial.print("    g: "); Serial.print(settings.g);
  Serial.print("    b: "); Serial.println(settings.b);
  Serial.print("bri: "); Serial.println(settings.bri);
  Serial.print("bri2: "); Serial.println(settings.bri2);
  Serial.print("rise: "); Serial.print(settings.rise);
  Serial.print("    fall: "); Serial.println(settings.fall);
  Serial.print("cylcle: "); Serial.println(settings.cycle);
}

void defaultSettings() {
  settings.on = true;
  settings.mode = 0;
  settings.r = 255;
  settings.g = 255;
  settings.b = 255;
  settings.bri = 256;
  settings.bri2 = 31;
  settings.rise = 1000;
  settings.fall = 1000;
  settings.cycle = 10000;
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
    EEPROM.get(sizeof(struct header), settings);
    settings.on = true;
    printSettings();
  }
}

void writeSettings() {
  EEPROM.put(0, expectedHeader);
  EEPROM.put(sizeof(struct header), settings);
  EEPROM.commit();
}
