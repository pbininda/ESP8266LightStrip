#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include "OTA.h"

extern "C" {
  #include "user_interface.h"
}

#include "SSID_PASSWORD.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const uint8 NUM_MODES = 10;
const uint16 NUM_LEDS = 106;
const bool DEBUG_HTTP = 0;
const bool DEBUG_TIMING = 0;
const int LED_PIN = 2;
bool wiFiSetupDone = false;

struct settings {
  uint8 on;
  uint8 mode;
  uint8 r;
  uint8 g;
  uint8 b;
  uint8 bri;
  long rise;
  long fall;
} settings;

struct state {
  time_t now;
  time_t riseStart;
  time_t riseStop;
  time_t fallStart;
  time_t fallStop;
  uint16 dynLevel;
  uint32 dynFactor;
  uint8 dynR;
  uint8 dynG;
  uint8 dynB;
  uint16 tick;
} state;

struct palette {
  uint8 r;
  uint8 g;
  uint8 b;
} palette[] = {
  {255, 255, 255},
  {255, 200, 120},
  {255, 0, 0},
  {0, 255, 0},
  {0, 0, 255},
  {255, 200, 0},
  {0, 255, 200},
  {255, 0, 200}
};
uint8 NUM_PALETTE = (sizeof palette) / (sizeof (struct palette));

uint8 briLevels[] = {4, 16, 64, 255};
uint8 NUM_BRILEVELS = (sizeof briLevels) / (sizeof (uint8));



ESP8266WebServer server(80);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ400);

#define TITEL "K&uuml;chenlicht"
uint8_t  MAC_STA[]                = {0,0,0,0,0,0};

void initWiFi() {
  // Connect to WiFi network
  Serial.print("MAC[STA]");
  uint8_t *MAC  = WiFi.macAddress(MAC_STA);                   //get MAC address of STA interface
  for (uint8_t i = 0; i < sizeof(MAC)+2; ++i){
    Serial.print(":");
    Serial.print(MAC[i],HEX);
    MAC_STA[i] = MAC[i];                                            //copy back to global variable
  }
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  
}

void initWiFi2() {
  if (!wiFiSetupDone) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
      Serial.println("initializing server\r\n");
      initServer();
      Serial.println("ready for commands\r\n");
      initOta();
      wiFiSetupDone = true;
    }
  }
}


void updateState() {
  state.now = millis();
  if (settings.on) {
    if (state.now >= state.riseStart && state.now < state.riseStop) {
      state.dynLevel = (state.now - state.riseStart) * NUM_LEDS / (state.riseStop - state.riseStart);
    }
    else if (state.now >= state.riseStop) {
      state.dynLevel = NUM_LEDS;
    }
  }
  else {
    if (state.now >= state.fallStart && state.now < state.fallStop) {
      state.dynLevel = (state.fallStop - state.now) * NUM_LEDS / (state.fallStop - state.fallStart);
    }
    else if (state.now >= state.fallStop) {
      state.dynLevel = 0;
    }
  }
  state.dynFactor = settings.bri;
  state.dynR = state.dynFactor * settings.r / 255;
  state.dynG = state.dynFactor * settings.g / 255;
  state.dynB = state.dynFactor * settings.b / 255;
}

void setLedsFixed(uint32_t c) {
  static uint32_t lastc;
  static uint32 lastLevel;
  if (c != lastc || state.dynLevel != lastLevel) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      if (i < state.dynLevel) {
        strip.setPixelColor(i, c);  
      }
      else {
        strip.setPixelColor(i, 0);
      }
    }
    strip.show();
    lastc = c;
    lastLevel = state.dynLevel;
  }
}

void setLedsZylon() {
  const uint16 swipeTime = 5000;
  const uint16 swipeHalfWidth = strip.numPixels() / 10;
  const uint16 briOff = 64;
  const uint16 swipeTimeHalf = swipeTime / 2;
  uint16 swipeTPos = state.now % swipeTime;
  if (swipeTPos > swipeTimeHalf) {
    swipeTPos -= swipeTimeHalf;
    swipeTPos = swipeTimeHalf - swipeTPos;
  }
  const sint16 swipePos = swipeTPos * strip.numPixels() / swipeTimeHalf;
  uint32 cLow = strip.Color(state.dynR * briOff / 256, state.dynG * briOff / 256, state.dynB * briOff / 256);
  uint32 cHigh = strip.Color(state.dynR, state.dynG, state.dynB);
  for (uint16 i = 0; i < strip.numPixels(); i++) {
    if (i < swipePos - swipeHalfWidth || i > swipePos + swipeHalfWidth) {
      strip.setPixelColor(i, cLow);
    }
    else {
      strip.setPixelColor(i, cHigh);
    }
  }
  strip.show();
}

void setLeds() {
  switch (settings.mode) {
    case 0:
      {
        setLedsFixed(strip.Color(state.dynR, state.dynG, state.dynB));
      }
      break;
    case 1:
      {
        setLedsZylon();
      }
      break;
    default:
      {
        setLedsFixed(strip.Color(0, 0, 0));
      }
      break;
  }
}

void loop() {
  static const int tickResolution = 1000;
  if (wiFiSetupDone) {
    server.handleClient();
  }
  else {
    initWiFi2();
  }
  updateState();
  setLeds();
  handleOta();
  if (settings.on == 0 && state.dynLevel == 0) {
    wifi_set_sleep_type(LIGHT_SLEEP_T);
    delay(200);
  }
  if (DEBUG_TIMING && state.tick % tickResolution == 0) {
    static time_t lastTickTime = 0;
    Serial.print("tick time: ");
    Serial.print((state.now - lastTickTime) / (tickResolution * 1.0));
    Serial.println("ms");
    lastTickTime = state.now;
  }
  state.tick++;
  delay(10);
  yield();
}

String head() {
  return "<!DOCTYPE html>\r\n<html>\r\n<head>\r\n<meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">\r\n<title>" TITEL "</title>\r\n</hread>\r\n<body>\r\n";
}

String tail() {
  return "</body>\r\n</html>\r\n";
}

void sendResult(String &resp) {
  server.send(200, "text/html", resp);
}

String statusBody() {
  String res("");
  if (state.riseStart || state.riseStop) {
    res += "<p>Rise time: " + String(state.riseStart - state.now) + " &rArr; " + String(state.riseStop - state.now) + "</p>";
  }
  if (state.fallStart || state.fallStop) {
    res += "<p>Rise time: " + String(state.fallStart - state.now) + " &rArr; " + String(state.fallStop - state.now) + "</p>";
  }
  res += "<p>Dyn Level: " + String(state.dynLevel) + "</p>";
  res += "<p>Dyn Factor: " + String(state.dynFactor) + "</p>";
  res += "<p>DynR: " + String(state.dynR);
  res += "   DynG: " + String(state.dynG);
  res += "   DynB: " + String(state.dynB) + "</p>";
  return res;
}


String numInput(const char *label, const char *name, long min, long max, int value) {
  return String("<label>") + label + ":</label><input name=\"" + name + "\" type=\"number\" min=\"" + String(min) + " \" max=\"" + String(max) + "\" value=\"" + String(value) + "\"></p>";
}

String formBody() {
  String res("<form action=\"/\" method=\"post\">");
  res += String("<p>") + numInput("On", "on", 0, 1, settings.on) + numInput("Mode", "mode", 0, NUM_MODES - 1, settings.mode) + "</p>";
  res += String("<p>") + numInput("R", "red", 0, 255, settings.r);
  res +=                 numInput("G", "green", 0, 255, settings.g);
  res +=                 numInput("B", "blue", 0, 255, settings.b) + "</p>";
  res +=         "<p>" + numInput("Brightness", "bri", 0, 255, settings.bri) + "</p>";
  res +=         "<p>" + numInput("Rise", "rise", 0, 10000, settings.rise);
  res +=                 numInput("Fall", "fall", 0, 10000, settings.fall) + "</p>";
  res +=         "<button type=\"submit\">Set</button>";
  res +=         "</form>\r\n";
  res += "<p><a href=\"/\">Reload</a></p>";
  return res;
}

bool extractArg(const char *arg, uint8 &target) {
  String str = server.arg(arg);
  if (str.length() > 0) {
    target = str.toInt();
    return true;
  }
  return false;
}

bool extractArgL(const char *arg, long &target) {
  String str = server.arg(arg);
  if (str.length() > 0) {
    target = str.toInt();
    return true;
  }
  return false;
}

void extractArgs() {
  bool wasOn = settings.on;
  extractArg("on", settings.on);
  extractArg("mode", settings.mode);
  extractArg("red", settings.r);
  extractArg("green", settings.g);
  extractArg("blue", settings.b);
  extractArg("bri", settings.bri);
  uint8 pal;
  if(extractArg("pal", pal)) {
    pal = pal % NUM_PALETTE;
    settings.r = palette[pal].r;
    settings.g = palette[pal].g;
    settings.b = palette[pal].b;
  }
  uint8 bril;
  if(extractArg("bril", bril)) {
    bril = bril % NUM_BRILEVELS;
    settings.bri = briLevels[bril];
  }
  time_t now = millis();
  extractArgL("rise", settings.rise);
  extractArgL("fall", settings.fall);
  if (settings.on  != wasOn) {
    if (settings.on) {
      state.riseStart = now;
      state.riseStop = now + settings.rise;
    }
    else {
      state.fallStart = now;
      state.fallStop = now + settings.fall;
    }
  }
  writeSettings();
}

void handleIndex() {
  extractArgs();
  sendResult(head() + formBody() + statusBody() + tail());
}


void handleSet() {
  extractArgs();
  sendResult(head() + formBody() + statusBody() + "<p>sent command</p>\r\n" + tail());
}

void initServer() {
  // Start the server
  server.on("/", HTTP_GET, handleIndex);
  server.on("/", HTTP_POST, handleSet);
  server.on("/switch", HTTP_GET, handleSet);
  server.begin();
  Serial.print("Server started on ");
  Serial.println(WiFi.localIP());
}

void initLed() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void readSettings() {
  EEPROM.begin(512);
  settings.on = true;
  settings.mode = EEPROM.read(0);
  settings.r = EEPROM.read(1);
  settings.g = EEPROM.read(2);
  settings.b = EEPROM.read(3);
  settings.bri = EEPROM.read(4);
  EEPROM.get(8, settings.rise);
  EEPROM.get(16, settings.fall);
}

void writeSettings() {
  EEPROM.write(0, settings.mode);
  EEPROM.write(1, settings.r);
  EEPROM.write(2, settings.g);
  EEPROM.write(3, settings.b);
  EEPROM.write(4, settings.bri);
  EEPROM.put(8, settings.rise);
  EEPROM.put(16, settings.fall);
  EEPROM.commit();
}

void initState() {
  state.dynLevel = 255;
  state.now = millis();
  state.riseStart = state.now;
  state.riseStop = state.now + settings.rise;
  state.dynLevel = 10;
  setLeds();
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("initializing LED Strip\r\n");
  readSettings();
  initState();
  initLed();
  Serial.println("initializing WIFI\r\n");
  initWiFi();
}


