#include <Arduino.h>
#include <EEPROM.h>
#include "state.h"
#include "persistence.h"

void readSettings() {
  EEPROM.begin(512);
  settings.on = true;
  settings.mode = EEPROM.read(0);
  settings.r = EEPROM.read(1);
  settings.g = EEPROM.read(2);
  settings.b = EEPROM.read(3);
  EEPROM.get(4, settings.bri);
  EEPROM.get(8, settings.rise);
  EEPROM.get(16, settings.fall);
  EEPROM.get(24, settings.cycle);
}

void writeSettings() {
  EEPROM.write(0, settings.mode);
  EEPROM.write(1, settings.r);
  EEPROM.write(2, settings.g);
  EEPROM.write(3, settings.b);
  EEPROM.put(4, settings.bri);
  EEPROM.put(8, settings.rise);
  EEPROM.put(16, settings.fall);
  EEPROM.put(24, settings.cycle);
  EEPROM.commit();
}


