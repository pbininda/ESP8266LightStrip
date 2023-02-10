#include <stdint.h>
#include <EEPROM.h>
#include "state.h"
#include "persistence.h"
#include "values.h"

struct header { // NOLINT(altera-struct-pack-align)
  uint16_t magic;
  uint16_t version;
  uint16_t num_strips;
}; // NOLINT(altera-struct-pack-align)

static const uint16_t MAGIC_NUMBER = 0x1ED5;

static struct header expectedHeader = {MAGIC_NUMBER, SW_VERSION_NO, NUM_STRIPS};

static void printSettings() {
  uint8_t count = 0;
  for (const Settings &settings: strip_settings) {
    Serial.print("Settings for strip "); Serial.println(count);
    Serial.print("on: "); Serial.print(settings.on);
    Serial.print("    mode: "); Serial.println(settings.mode);
    Serial.print("r: "); Serial.print(settings.palette[settings.colidx].red);
    Serial.print("    g: "); Serial.print(settings.palette[settings.colidx].green);
    Serial.print("    b: "); Serial.println(settings.palette[settings.colidx].blue);
    Serial.print("bri: "); Serial.println(settings.bri);
    Serial.print("bri2: "); Serial.println(settings.bri2);
    Serial.print("rise: "); Serial.print(settings.rise);
    Serial.print("    fall: "); Serial.println(settings.fall);
    Serial.print("cylcle: "); Serial.println(settings.cycle);
    Serial.print("onoffmode: "); Serial.println(settings.onoffmode);
    count = count + 1;
  }
}

static void defaultSettings() {
  for (Settings &settings: strip_settings) {
    settings.on = 1;
    settings.mode = 0;
    settings.colidx = 0;
    settings.bri = NUM_IN_BYTE;
    settings.bri2 = DEC;
    settings.rise = ONE_S_IN_MS;
    settings.fall = ONE_S_IN_MS;
    settings.cycle = TEN_S_IN_MS;
    settings.onoffmode = 0;
    settings.ngradient = 1;
    settings.palette[0] = {255, 255, 255}; // NOLINT
    settings.palette[1] = {255, 200, 120}; // NOLINT
    settings.palette[2] = {255, 180, 100}; // NOLINT
    settings.palette[3] = {255, 120, 50}; // NOLINT
    settings.palette[4] = {255, 100, 40}; // NOLINT
    settings.palette[5] = {255, 80, 30}; // NOLINT
    settings.palette[6] = {255, 60, 20}; // NOLINT
    settings.palette[7] = {255, 0, 0}; // NOLINT
    settings.palette[8] = {255, 255, 255}; // NOLINT
    settings.palette[9] = {255, 200, 120}; // NOLINT
    settings.palette[10] = {255, 180, 100}; // NOLINT
    settings.palette[11] = {255, 120, 50}; // NOLINT
    settings.palette[12] = {255, 100, 40}; // NOLINT
    settings.palette[13] = {255, 80, 30}; // NOLINT
    settings.palette[14] = {255, 60, 20}; // NOLINT
    settings.palette[15] = {255, 0, 0}; // NOLINT
    settings.palette[16] = {255, 255, 255}; // NOLINT
    settings.palette[17] = {255, 200, 120}; // NOLINT
    settings.palette[18] = {255, 180, 100}; // NOLINT
    settings.palette[19] = {255, 120, 50}; // NOLINT
    settings.palette[20] = {255, 100, 40}; // NOLINT
    settings.palette[21] = {255, 80, 30}; // NOLINT
    settings.palette[22] = {255, 60, 20}; // NOLINT
    settings.palette[23] = {255, 0, 0}; // NOLINT
  }
}

static const int EEPROM_SPACE_RESERVED = 512;

void readSettings() { // cppcheck-suppress unusedFunction
  Serial.println(String("EEPROM space ") + String(sizeof(header) + sizeof(strip_settings)) + String(" / ") + String(EEPROM_SPACE_RESERVED));
  EEPROM.begin(EEPROM_SPACE_RESERVED);
  struct header header; // NOLINT
  EEPROM.get(0, header);
  if (header.magic != expectedHeader.magic) {
    Serial.println("header magic mismatch");
    defaultSettings();
    printSettings();
    writeSettings();
    return;
  }
  if (header.version != expectedHeader.version) {
    Serial.println("header version mismatch");
    defaultSettings();
    printSettings();
    writeSettings();
    return;
  }
  if (header.num_strips != expectedHeader.num_strips) {
    Serial.println("header num strips mismatch");
    defaultSettings();
    printSettings();
    writeSettings();
    return;
  }
  EEPROM.get(sizeof(struct header), strip_settings);
  for (Settings &settings: strip_settings) {
    settings.on = 1;
  }
  printSettings();
}

void writeSettings() {
  EEPROM.put(0, expectedHeader);
  EEPROM.put(sizeof(struct header), strip_settings);
  EEPROM.commit();
}
