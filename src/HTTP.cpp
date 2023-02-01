#include <stdint.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "settings.h"
#include "HTTP.h"
#include "state.h"
#include "persistence.h"
#include "LED.h"
#include "WiFiServe.h"
#include "index.h"

static WebServer server(80);
static bool serverSetupDone = false;
static const uint8_t NUM_MODES = 10;

static const char HEAD_1[] PROGMEM = "<!DOCTYPE html>\r\n<html>\r\n<head>\r\n<meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">\r\n<title>";
static const char HEAD_2[] PROGMEM = "</title>\r\n</hread>\r\n<body>\r\n";

static String head(const StripSettings &stripSettings) {
  return String(HEAD_1) + stripSettings.STRIP_NAME + HEAD_2;
}

static String tail() {
  return "</body>\r\n</html>\r\n";
}

static void sendResult(const String &resp) {
  server.send(200, "text/html", resp);
}

static void sendJsonResult(const String resp) {
  Serial.println(String("StackSize in send json: ") + uxTaskGetStackHighWaterMark(NULL));
  server.send(200, "application/json", resp);
}

static String statusBody(const Settings &settings, const State &state,const Led &led, const Effects &effects) {
  String res("");
  res += "<p>V2</p>";
  if (state.riseStart || state.riseStop) {
    res += "<p>Rise time: " + String(state.riseStart - state.now) + " &rArr; " + String(state.riseStop - state.now) + "</p>";
  }
  if (state.fallStart || state.fallStop) {
    res += "<p>Fall time: " + String(state.fallStart - state.now) + " &rArr; " + String(state.fallStop - state.now) + "</p>";
  }
  res += "<p>Dyn Level: " + String(state.dynLevel) + "</p>";
  res += "<p>Dyn Factor: " + String(state.dynFactor) + "</p>";
  struct palette p = effects.dynGradColor(0);
  res += "<p>DynR: " + String(p.r);
  res += "   DynG: " + String(p.g);
  res += "   DynB: " + String(p.b) + "</p>";
  res += "<p>Led1: " + String(led.getLed(1), HEX) + "</p>";
  return res;
}

static String numInput(const char *label, const char *name, long min, long max, int value) {
  return String("<label>") + label + ":</label><input name=\"" + name + "\" type=\"number\" min=\"" + String(min) + " \" max=\"" + String(max) + "\" value=\"" + String(value) + "\"></p>";
}

static String formBody(const Settings &settings, int8_t strip) {
  String strp = strip < 0 ? "" : String(strip);
  String res("<form action=\"/{{STRIP}}\" method=\"post\">");
  res += String("<p>") + numInput("On", "on", 0, 1, settings.on) + numInput("Mode", "mode", 0, NUM_MODES - 1, settings.mode) + numInput("OnOff Mode", "onoffmode", 0, ONOFFMODE_LAST - 1, settings.onoffmode) + "</p>";
  res +=         "<p>" + numInput("Color Index", "colidx", 0, NUM_PALETTE - 1, settings.colidx) + "</p>";
  res +=         "<p>" + numInput("Gradient Length", "ngradient", 0, NUM_PALETTE - 1, settings.ngradient) + "</p>";
  res +=         "<p>" + numInput("Brightness", "bri", 0, 256, settings.bri) + "</p>";
  res +=         "<p>" + numInput("Brightness2", "bri2", 0, 256, settings.bri2) + "</p>";
  res +=         "<p>" + numInput("Cycle", "cycle", 0, 60000, settings.cycle) + "</p>";
  res +=         "<p>" + numInput("Rise", "rise", 0, 10000, settings.rise);
  res +=                 numInput("Fall", "fall", 0, 10000, settings.fall) + "</p>";
  res +=         "<button type=\"submit\">Set</button>";
  res +=         "</form>\r\n";
  res += "<p><a href=\"/{{STRIP}}\">Reload</a></p>\r\n";
  res += "<form action=\"/setup\" method=\"post\">";
  res += "<button type=\"submit\">Reset WiFi</button>";
  res += "</form>\r\n";
  res.replace("{{STRIP}}", strp);
  return res;
}

static bool extractArg8(const char *arg, uint8_t &target) {
  String str = server.arg(arg);
  if (str.length() > 0) {
    target = str.toInt();
    return true;
  }
  return false;
}

static bool extractArg16(const char *arg, uint16_t &target) {
  String str = server.arg(arg);
  if (str.length() > 0) {
    target = str.toInt();
    return true;
  }
  return false;
}

static bool extractArg32(const char *arg, uint32_t &target) {
  String str = server.arg(arg);
  if (str.length() > 0) {
    target = str.toInt();
    return true;
  }
  return false;
}

static void processSettings(const Settings &settings, State &state, bool wasOn) {
  time_t now = millis();
  if (settings.on != wasOn) {
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

static void extractArgs(Settings &settings, State &state) {
  bool wasOn = settings.on;
  extractArg8("on", settings.on);
  extractArg8("mode", settings.mode);
  extractArg8("onoffmode", settings.onoffmode);
  extractArg8("colidx", settings.colidx);
  extractArg8("ngradient", settings.ngradient);
  extractArg16("bri", settings.bri);
  extractArg8("bri2", settings.bri2);
  extractArg32("cycle", settings.cycle);
  uint8_t bril;
  if(extractArg8("bril", bril)) {
    bril = bril % NUM_BRILEVELS;
    settings.bri = briLevels[bril];
  }
  extractArg32("rise", settings.rise);
  extractArg32("fall", settings.fall);
  processSettings(settings, state, wasOn);
}

static void handleIndex(Settings &settings, State &state, const Led &led, const Effects &effects, const StripSettings &stripSettings, uint8_t strip) {
  extractArgs(settings, state);
  sendResult(head(stripSettings) + formBody(settings, strip) + statusBody(settings, state, led, effects) + tail());
}

static void handleSet(Settings &settings, State &state, const Led &led, const Effects &effects, const StripSettings &stripSettings, uint8_t strip, bool sendRes) {
  extractArgs(settings, state);
  if (sendRes) {
    sendResult(head(stripSettings) + formBody(settings, strip) + statusBody(settings, state, led, effects) + "<p>sent command</p>\r\n" + tail());
  }
}

static void handleApiGet(const Settings &settings, const State &state, const Effects &effects) {
  DynamicJsonDocument jsonDocument(2536);
  JsonObject jsRoot = jsonDocument.to<JsonObject>();
  JsonObject jsState = jsRoot.createNestedObject("state");  
  JsonObject jsSettings = jsRoot.createNestedObject("settings");
  JsonArray jsPalette = jsRoot.createNestedArray("palette");

  jsSettings["on"] = settings.on != 0;
  jsSettings["mode"] = settings.mode;
  jsSettings["colidx"] = settings.colidx;
  jsSettings["ngradient"] = settings.ngradient;
  jsSettings["bri"] = settings.bri;
  jsSettings["bri2"] = settings.bri2;
  jsSettings["cycle"] = settings.cycle;
  jsSettings["rise"] = settings.rise;
  jsSettings["fall"] = settings.fall;
  
  jsState["riseStart"] = state.riseStart;
  jsState["riseStop"] = state.riseStop;
  jsState["fallStart"] = state.fallStart;
  jsState["fallStop"] = state.fallStop;
  jsState["tick"] = state.tick;
  jsState["now"] = state.now;
  jsState["dynLevel"] = state.dynLevel;
  jsState["dynFactor"] = state.dynFactor;

  struct palette p = effects.dynGradColor(0);
  jsState["dynR"] = p.r;
  jsState["dynG"] = p.g;
  jsState["dynB"] = p.b;

  for (int i = 0; i < NUM_PALETTE; i ++) {
    JsonObject c = jsPalette.createNestedObject();
    c["r"] = settings.palette[i].r;
    c["g"] = settings.palette[i].g;
    c["b"] = settings.palette[i].b;
  }


  String jsonString;
  serializeJson(jsonDocument, jsonString);
  // Serial.println(jsonString);
  sendJsonResult(jsonString);
}

static void handleApiPost(Settings &settings, State &state, const StripSettings &stripSettings, bool sendResult) {
  Serial.println(String("StackSize at start of post: ") + uxTaskGetStackHighWaterMark(NULL));
  bool wasOn = settings.on;
  DynamicJsonDocument jsonDocument(2536);
  String jsonString(server.arg("plain"));
  Serial.print("POST: ");
  Serial.println(jsonString);
  DeserializationError error = deserializeJson(jsonDocument, jsonString);
  String res = "OK";
  if (error) {
    Serial.println(error.c_str());
    res = String("FAIL ") + error.c_str();
  } else {
    JsonObject root = jsonDocument.as<JsonObject>();
    JsonObject jsSettings = root["settings"];
    if (jsSettings.containsKey("on")) {
      bool on = jsSettings["on"];
      Serial.println(String("on: ") + on);
      settings.on = on;
      res = "SETON OK";
    }
    if (jsSettings.containsKey("bri")) {
      settings.bri = jsSettings["bri"];
      settings.bri %= 257;
    }
    if (jsSettings.containsKey("bri2")) {
      settings.bri2 = jsSettings["bri2"];
      if (settings.bri2 > stripSettings.MAX_BRI2) {
        settings.bri2 = stripSettings.MAX_BRI2;
      }
      settings.bri2 %= 32;
    }
    if (jsSettings.containsKey("colidx")) {
      settings.colidx = jsSettings["colidx"];
      settings.colidx %= NUM_PALETTE;
    }
    if (jsSettings.containsKey("ngradient")) {
      settings.ngradient = jsSettings["ngradient"];
      settings.ngradient %= NUM_PALETTE;
      if (settings.ngradient < 1) {
        settings.ngradient = 1;
      }
    }
    if (jsSettings.containsKey("setpal")) {
      JsonObject jsSetPal = jsSettings["setpal"];
      if (jsSetPal.containsKey("idx")) {
        uint8_t idx = jsSetPal["idx"];
        if (idx < NUM_PALETTE) {
          uint8_t r = jsSetPal["r"];
          uint8_t g = jsSetPal["g"];
          uint8_t b = jsSetPal["b"];
          settings.palette[idx] = {r, g, b};
        }
      }
    }
    if (jsSettings.containsKey("mode")) {
      settings.mode = jsSettings["mode"];
    }
  }
  processSettings(settings, state, wasOn);
  Serial.println(String("StackSize at end of post: ") + uxTaskGetStackHighWaterMark(NULL));
  if (sendResult) {
    sendJsonResult(String("\"") + res + "\"");
  }
}

static void handleLedsGet(const Led &led) {
  DynamicJsonDocument jsonDocument(5536);
  JsonObject jsRoot = jsonDocument.to<JsonObject>();
  JsonArray jsLeds = jsRoot.createNestedArray("leds");
  for (uint16_t i = 0; i < led.stripSettings.NUM_LEDS; i ++) {
    JsonObject c = jsLeds.createNestedObject();
    INTERNAL_RGBW l = led.getLedc(i);
    c["r"] = l.r;
    c["g"] = l.g;
    c["b"] = l.b;
    c["w"] = l.w;
  }
  String jsonString;
  serializeJson(jsonDocument, jsonString);
  // Serial.println(jsonString);
  sendJsonResult(jsonString);
}

static String index() {
  String res(HTTP_MAIN);
  return res;
}

static void handleSpa(Settings &settings, State &state, const StripSettings &stripSettings, int8_t strip) {
  String strp = strip < 0 ? "" : String("/") + String(strip);
  extractArgs(settings, state);
  String indexData = index();
  indexData.replace("{{SYSTEM_NAME}}", stripSettings.STRIP_NAME);
  indexData.replace("{{STRIP}}", strp);
  sendResult(indexData);
}

static void handleSetupRedir() {
  server.sendHeader("location", "/");
  server.send(301, "application/text", "redirect to root");
}

static void handleSetup() {
  handleSetupRedir();
  startWiFiPortal();
}


void initServer(Settings *settings, State *state, Led **led, Effects **effects) {
  // Start the server
  server.on("/", HTTP_GET, [=]() { handleIndex(settings[0], state[0], *led[0], *effects[0], STRIP_SETTINGS[0], -1); });
  server.on("/", HTTP_POST, [=]() { 
    for (uint8_t i = 0; i < NUM_STRIPS; i++) {
      handleSet(settings[i], state[i], *led[i], *effects[i], STRIP_SETTINGS[i], -1, i == NUM_STRIPS - 1);
    }
  });
  server.on("/spa", HTTP_GET, [=]() { handleSpa(settings[0], state[0], STRIP_SETTINGS[0], -1); });
  server.on("/api", HTTP_GET, [=]() { handleApiGet(settings[0], state[0], *effects[0]); });
  server.on("/api", HTTP_POST, [=]() { 
    for (uint8_t i = 0; i < NUM_STRIPS; i++) {
      handleApiPost(settings[i], state[i], STRIP_SETTINGS[i], i == NUM_STRIPS - 1);
    }
  });
  server.on("/leds", HTTP_GET, [=]() { handleLedsGet(*led[0]); });
  server.on("/setup", HTTP_POST, handleSetup);
  server.on("/setup", HTTP_GET, handleSetupRedir);
  Serial.println(String("Setting up ") + String(NUM_STRIPS) + " strip routes");
  for (uint8_t i = 0; i < NUM_STRIPS; i++) {
    String strip(i);
    Serial.println("Setting routes for /" + strip);
    server.on("/" + strip, HTTP_GET, [=]() { handleIndex(settings[i], state[i], *led[i], *effects[i], STRIP_SETTINGS[i], i); });
    server.on("/" + strip, HTTP_POST, [=]() { handleSet(settings[i], state[i], *led[i], *effects[i], STRIP_SETTINGS[i], i, true); });
    server.on("/spa/" + strip, HTTP_GET, [=]() { handleSpa(settings[i], state[i], STRIP_SETTINGS[i], i); });
    server.on("/api/" + strip, HTTP_GET, [=]() { handleApiGet(settings[i], state[i], *effects[i]); });
    server.on("/api/" + strip, HTTP_POST, [=]() { handleApiPost(settings[i], state[i], STRIP_SETTINGS[i], true); });
    server.on("/leds/" + strip, HTTP_GET, [=]() { handleLedsGet(*led[i]); });
  }
  Serial.println("Route Setup done");
  server.begin();
  serverSetupDone = true;
  Serial.print("Server started on ");
  Serial.println(WiFi.localIP());
}

void handleServer() {
  if (serverSetupDone) {
      server.handleClient();
  }
}
