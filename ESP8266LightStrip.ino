#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

extern "C" {
  #include "user_interface.h"
}

#include "SSID_PASSWORD.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const uint8 NUM_MODES = 10;
const uint16 NUM_LEDS = 106;
const bool DEBUG_HTTP = 0;
const int LED_PIN = 2;
bool wiFiSetupDone = false;
bool otaSetupDone = false;

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

ESP8266WebServer server(80);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define TITEL "K&uuml;chenlicht"

void initWiFi() {
  // Connect to WiFi network
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
      state.dynLevel = (state.now - state.riseStart) * 256 / (state.riseStop - state.riseStart);
    }
    else if (state.now >= state.riseStop) {
      state.dynLevel = 256;
    }
  }
  else {
    if (state.now >= state.fallStart && state.now < state.fallStop) {
      state.dynLevel = (state.fallStop - state.now) * 256 / (state.fallStop - state.fallStart);
    }
    else if (state.now >= state.fallStop) {
      state.dynLevel = 0;
    }
  }
  state.dynFactor = state.dynLevel * settings.bri;
  state.dynR = state.dynFactor * settings.r / 256 / 255;
  state.dynG = state.dynFactor * settings.g / 256 / 255;
  state.dynB = state.dynFactor * settings.b / 256 / 255;
}

void setLedsFixed(uint32_t c) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);  
  }
}

void setLeds() {
  switch (settings.mode) {
    case 0:
      {
        setLedsFixed(strip.Color(state.dynR, state.dynG, state.dynB));
      }
      break;
    default:
      {
        setLedsFixed(strip.Color(0, 0, 0));
      }
      break;
  }
  strip.show();
}

void loop() {
  state.tick++;
  if (wiFiSetupDone) {
    server.handleClient();
  }
  else {
    initWiFi2();
  }
  updateState();
  setLeds();
  /*
  if (settings.on == 0 && state.dynFactor == 0) {
    wifi_set_sleep_type(LIGHT_SLEEP_T);
    delay(200);
  }
  */
  if (state.tick % 1000 == 0) {
    Serial.println("tick");
  }
  if (otaSetupDone) {
    ArduinoOTA.handle();
  }
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

void initOta() {
  String hostname("ESP8266-OTA-");
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);
  Serial.println("Hostname: " + hostname);
  // Start OTA server.
  ArduinoOTA.setHostname((const char *)hostname.c_str());
  ArduinoOTA.setPassword((const char *)"123");
  ArduinoOTA.onStart([]() {
    /*
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
    */
    Serial.println("Start updating");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  otaSetupDone = true;
  Serial.println("ready for OTA");
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


