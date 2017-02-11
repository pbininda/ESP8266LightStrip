#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

extern "C" {
  #include "user_interface.h"
}

#include "SSID_PASSWORD.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const uint8 NUM_MODES = 10;
const uint8 NUM_LEDS = 5;

struct settings {
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
} state;

ESP8266WebServer server(80);
const bool DEBUG_HTTP = 0;
long commandsSent = 0;
const int pin = 2;

#define TITEL "K&uuml;chenlicht"

void initWiFi() {
  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
}



void updateState() {
  state.now = millis();
  if (state.now > state.riseStop) {
    state.riseStart = state.riseStop = 0;
  }
  if (state.now > state.fallStop) {
    state.fallStart = state.fallStop = 0;
  }
  if (state.now >= state.riseStart && state.now < state.riseStop) {
    state.dynLevel = (state.now - state.riseStart) * 256 / (state.riseStop - state.riseStart);
  }
  else if (state.now >= state.fallStart && state.now < state.fallStop) {
    state.dynLevel = (state.fallStop - state.now) * 256 / (state.fallStop - state.fallStart);
  }
}

void loop() {
  server.handleClient();
  updateState();
  /*
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  delay(200);*/
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
  String res = "<p>Commands sent: ";
  res += commandsSent + "</p>";
  if (state.riseStart || state.riseStop) {
    res += "<p>Rise time: " + String(state.riseStart - state.now) + " &rArr; " + String(state.riseStop - state.now) + "</p>";
  }
  if (state.fallStart || state.fallStop) {
    res += "<p>Rise time: " + String(state.fallStart - state.now) + " &rArr; " + String(state.fallStop - state.now) + "</p>";
  }
  res += "<p>Dyn Level: " + String(state.dynLevel) + "</p>";
  return res;
}


String numInput(const char *label, const char *name, long min, long max, int value) {
  return String("<label>") + label + ":</label><input name=\"" + name + "\" type=\"number\" min=\"" + String(min) + " \" max=\"" + String(max) + "\" value=\"" + String(value) + "\"></p>";
}

String formBody() {
  String res("<form action=\"/\" method=\"post\">");
  res += String("<p>") + numInput("Mode", "mode", 0, NUM_MODES - 1, settings.mode) + "</p>";
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
  extractArg("mode", settings.mode);
  extractArg("red", settings.r);
  extractArg("green", settings.g);
  extractArg("blue", settings.b);
  extractArg("bri", settings.bri);
  time_t now = millis();
  if (extractArgL("rise", settings.rise) && settings.rise > 0) {
    state.riseStart = now;
    state.riseStop = now + settings.rise;
  }
  if (extractArgL("fall", settings.fall) && settings.fall > 0) {
    time_t then = now;
    if (state.riseStop > then) {
      then = state.riseStop;
    }
    state.fallStart = then;
    state.fallStop = then + settings.fall;
  }
}

void handleIndex() {
  extractArgs();
  sendResult(head() + formBody() + statusBody() + tail());
}

void handleSet() {
  extractArgs();
  //stripSend();
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
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void initSettings() {
  settings.mode = 0;
  settings.r = settings.g = settings.b = 0;
  settings.bri = 0;
  settings.rise = settings.fall = 0;
}

void initState() {
  state.dynLevel = 255;
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("initializing LED Strip\r\n");
  initSettings();
  initState();
  initLed();
  Serial.println("initializing WIFI\r\n");
  initWiFi();
  Serial.println("initializing server\r\n");
  initServer();
  Serial.println("ready for commands\r\n");
}


