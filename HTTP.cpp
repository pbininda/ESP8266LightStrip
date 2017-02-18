#include <ESP8266WebServer.h>
#include "HTTP.h"
#include "state.h"
#include "persistence.h"

ESP8266WebServer server(80);
bool serverSetupDone = false;
const uint8 NUM_MODES = 10;

#define TITEL "K&uuml;chenlicht"

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
  serverSetupDone = true;
  Serial.print("Server started on ");
  Serial.println(WiFi.localIP());
}

void handleServer() {
  if (serverSetupDone) {
      server.handleClient();
  }
}

