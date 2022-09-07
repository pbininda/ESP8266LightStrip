#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "HTTP.h"
#include "state.h"
#include "persistence.h"
#include "index.h"

ESP8266WebServer server(80);
bool serverSetupDone = false;
const uint8 NUM_MODES = 10;
const uint8 MAX_BRI2 = 24;
#define TITEL "Badlicht"

String head() {
  return "<!DOCTYPE html>\r\n<html>\r\n<head>\r\n<meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">\r\n<title>" TITEL "</title>\r\n</hread>\r\n<body>\r\n";
}

String tail() {
  return "</body>\r\n</html>\r\n";
}

void sendResult(const String &resp) {
  server.send(200, "text/html", resp);
}

void sendJsonResult(const String resp) {
  server.send(200, "application/json", resp);
}

extern uint32_t getLed(uint16_t n);

String statusBody() {
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
  res += "<p>DynR: " + String(state.dynR);
  res += "   DynG: " + String(state.dynG);
  res += "   DynB: " + String(state.dynB) + "</p>";
  res += "<p>Led1: " + String(getLed(1), HEX) + "</p>";
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
  res +=         "<p>" + numInput("Brightness", "bri", 0, 256, settings.bri) + "</p>";
  res +=         "<p>" + numInput("Brightness2", "bri2", 0, 256, settings.bri2) + "</p>";
  res +=         "<p>" + numInput("Cycle", "cycle", 0, 60000, settings.cycle) + "</p>";
  res +=         "<p>" + numInput("Rise", "rise", 0, 10000, settings.rise);
  res +=                 numInput("Fall", "fall", 0, 10000, settings.fall) + "</p>";
  res +=         "<button type=\"submit\">Set</button>";
  res +=         "</form>\r\n";
  res += "<p><a href=\"/\">Reload</a></p>";
  return res;
}

bool extractArg8(const char *arg, uint8_t &target) {
  String str = server.arg(arg);
  if (str.length() > 0) {
    target = str.toInt();
    return true;
  }
  return false;
}

bool extractArg16(const char *arg, uint16_t &target) {
  String str = server.arg(arg);
  if (str.length() > 0) {
    target = str.toInt();
    return true;
  }
  return false;
}

bool extractArg32(const char *arg, uint32_t &target) {
  String str = server.arg(arg);
  if (str.length() > 0) {
    target = str.toInt();
    return true;
  }
  return false;
}

void processSettings(bool wasOn) {
  time_t now = millis();
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

void extractArgs() {
  bool wasOn = settings.on;
  extractArg8("on", settings.on);
  extractArg8("mode", settings.mode);
  extractArg8("red", settings.r);
  extractArg8("green", settings.g);
  extractArg8("blue", settings.b);
  extractArg16("bri", settings.bri);
  extractArg8("bri2", settings.bri2);
  extractArg32("cycle", settings.cycle);
  uint8 pal;
  if(extractArg8("pal", pal)) {
    pal = pal % NUM_PALETTE;
    settings.r = palette[pal].r;
    settings.g = palette[pal].g;
    settings.b = palette[pal].b;
  }
  uint8 bril;
  if(extractArg8("bril", bril)) {
    bril = bril % NUM_BRILEVELS;
    settings.bri = briLevels[bril];
  }
  extractArg32("rise", settings.rise);
  extractArg32("fall", settings.fall);
  processSettings(wasOn);
}

void handleIndex() {
  extractArgs();
  sendResult(head() + formBody() + statusBody() + tail());
}


void handleSet() {
  extractArgs();
  sendResult(head() + formBody() + statusBody() + "<p>sent command</p>\r\n" + tail());
}

void handleApiGet() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject &jsRoot = jsonBuffer.createObject();
  JsonObject &jsState = jsRoot.createNestedObject("state");  
  JsonObject &jsSettings = jsRoot.createNestedObject("settings");
  JsonArray &jsPalette = jsRoot.createNestedArray("palette");

  jsSettings["on"] = settings.on != 0;
  jsSettings["mode"] = settings.mode;
  jsSettings["r"] = settings.r;
  jsSettings["g"] = settings.g;
  jsSettings["b"] = settings.b;
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
  jsState["dynR"] = state.dynR;
  jsState["dynG"] = state.dynG;
  jsState["dynB"] = state.dynB;

  for (int i = 0; i < NUM_PALETTE; i ++) {
    JsonObject &c = jsPalette.createNestedObject();
    c["r"] = palette[i].r;
    c["g"] = palette[i].g;
    c["b"] = palette[i].b;
  }


  String jsonString;
  jsRoot.printTo(jsonString);
  // Serial.println(jsonString);
  sendJsonResult(jsonString);
}

void handleApiPost() {
  Serial.println("got post");
  bool wasOn = settings.on;
  DynamicJsonBuffer jsonBuffer;
  String jsonString(server.arg("plain"));
  Serial.print("POST: ");
  Serial.println(jsonString);
  JsonObject &root = jsonBuffer.parseObject(jsonString);
  if (root.success()) {
    // Serial.println("root success");
    // JsonObject &jsState = root["state"];
    JsonObject &jsSettings = root["settings"];
    if (jsSettings.containsKey("on")) {
      bool on = jsSettings["on"];
      Serial.println(String("on: ") + on);
      settings.on = on;
    }
    if (jsSettings.containsKey("bri")) {
      settings.bri = jsSettings["bri"];
      settings.bri %= 257;
    }
    if (jsSettings.containsKey("bri2")) {
      settings.bri2 = jsSettings["bri2"];
      if (settings.bri2 > MAX_BRI2) {
        settings.bri2 = MAX_BRI2;
      }
      settings.bri2 %= 32;
    }
    if (jsSettings.containsKey("r")) {
      settings.r = jsSettings["r"];
      settings.r %= 256;
    }
    if (jsSettings.containsKey("g")) {
      settings.g = jsSettings["g"];
      settings.g %= 256;
    }
    if (jsSettings.containsKey("b")) {
      settings.b = jsSettings["b"];
      settings.b %= 256;
    }
    if (jsSettings.containsKey("pal")) {
      uint32 pal = jsSettings["pal"];
      pal = pal % NUM_PALETTE;
      settings.r = palette[pal].r;
      settings.g = palette[pal].g;
      settings.b = palette[pal].b;    
    }
    if (jsSettings.containsKey("mode")) {
      settings.mode = jsSettings["mode"];
    }
  }
  sendJsonResult("\"OK\"");
  processSettings(wasOn);
}

String index() {
  String res(HTTP_MAIN);
  return res;
}

void handleSpa() {
  extractArgs();
  String indexData = index();
  sendResult(indexData);
}


void initServer() {
  // Start the server
  server.on("/", HTTP_GET, handleIndex);
  server.on("/", HTTP_POST, handleSet);
  server.on("/spa", HTTP_GET, handleSpa);
  server.on("/switch", HTTP_GET, handleSet);
  server.on("/api", HTTP_GET, handleApiGet);
  server.on("/api", HTTP_POST, handleApiPost);
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
