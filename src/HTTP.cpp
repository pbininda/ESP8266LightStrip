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
#include "values.h"
#include "version.h"

static uint16_t briLevels[] = {4, 16, 64, 256}; // NOLINT
static uint8_t NUM_BRILEVELS = (sizeof briLevels) / (sizeof briLevels[0]);


static WebServer server(80);
static bool serverSetupDone = false;
static const uint8_t NUM_MODES = 10;

static const int HTTP_OK = 200;
static const int HTTP_REDIRECT = 301;

static const char HEAD_1[] PROGMEM = "<!DOCTYPE html>\r\n<html>\r\n<head>\r\n<meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">\r\n<title>";

static const char HEAD_2[] PROGMEM = "</title>" 
    "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"data:image/png;base64,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CiAgICB4bWxuczp0aWZmPSJodHRwOi8vbnMuYWRvYmUuY29tL3RpZmYvMS4wLyIKICAgIHhtbG5zOnhtcD0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wLyIKICAgeG1wTU06RG9jdW1lbnRJRD0iZ2ltcDpkb2NpZDpnaW1wOjM5NDM0ZjllLWNjMzktNDM2Yi1iMmNmLThmZjdkODAzMWE5YyIKICAgeG1wTU06SW5zdGFuY2VJRD0ieG1wLmlpZDoyMDk1Njc5My1mYTdmLTQyZjgtODY0Yy02N2I2OTE0OWMyZGMiCiAgIHhtcE1NOk9yaWdpbmFsRG9jdW1lbnRJRD0ieG1wLmRpZDpmN2RhZDllMS03MmFiLTQ3MGMtODM2YS1hN2M5ZmUyNmRjZDUiCiAgIEdJTVA6QVBJPSIyLjAiCiAgIEdJTVA6UGxhdGZvcm09IldpbmRvd3MiCiAgIEdJTVA6VGltZVN0YW1wPSIxNjc1NTQwMzEwOTc1NjI0IgogICBHSU1QOlZlcnNpb249IjIuMTAuMjIiCiAgIGRjOkZvcm1hdD0iaW1hZ2UvcG5nIgogICB0aWZmOk9yaWVudGF0aW9uPSIxIgogICB4bXA6Q3JlYXRvclRvb2w9IkdJTVAgMi4xMCI+CiAgIDxpcHRjRXh0OkxvY2F0aW9uQ3JlYXRlZD4KICAgIDxyZGY6QmFnLz4KICAgPC9pcHRjRXh0OkxvY2F0aW9uQ3JlYXRlZD4KICAgPGlwdGNFeHQ6TG9jYXRpb25TaG93bj4KICAgIDxyZGY6QmFnLz4KICAgPC9pcHRjRXh0OkxvY2F0aW9uU2hvd24+CiAgIDxpcHRjRXh0OkFydHdvcmtPck9iamVjdD4KICAgIDxyZGY6QmFnLz4KICAgPC9pcHRjRXh0OkFydHdvcmtPck9iamVjdD4KICAgPGlwdGNFeHQ6UmVnaXN0cnlJZD4KICAgIDxyZGY6QmFnLz4KICAgPC9pcHRjRXh0OlJlZ2lzdHJ5SWQ+CiAgIDx4bXBNTTpIaXN0b3J5PgogICAgPHJkZjpTZXE+CiAgICAgPHJkZjpsaQogICAgICBzdEV2dDphY3Rpb249InNhdmVkIgogICAgICBzdEV2dDpjaGFuZ2VkPSIvIgogICAgICBzdEV2dDppbnN0YW5jZUlEPSJ4bXAuaWlkOjdhNjQ5MjcyLWIwNjEtNDU4OS1iNmY2LTM4NDk4YTE4OTFhOSIKICAgICAgc3RFdnQ6c29mdHdhcmVBZ2VudD0iR2ltcCAyLjEwIChXaW5kb3dzKSIKICAgICAgc3RFdnQ6d2hlbj0iMjAyMy0wMi0wNFQyMDo1MTo1MCIvPgogICAgPC9yZGY6U2VxPgogICA8L3htcE1NOkhpc3Rvcnk+CiAgIDxwbHVzOkltYWdlU3VwcGxpZXI+CiAgICA8cmRmOlNlcS8+CiAgIDwvcGx1czpJbWFnZVN1cHBsaWVyPgogICA8cGx1czpJbWFnZUNyZWF0b3I+CiAgICA8cmRmOlNlcS8+CiAgIDwvcGx1czpJbWFnZUNyZWF0b3I+CiAgIDxwbHVzOkNvcHlyaWdodE93bmVyPgogICAgPHJkZjpTZXEvPgogICA8L3BsdXM6Q29weXJpZ2h0T3duZXI+CiAgIDxwbHVzOkxpY2Vuc29yPgogICAgPHJkZjpTZXEvPgogICA8L3BsdXM6TGljZW5zb3I+CiAgPC9yZGY6RGVzY3JpcHRpb24+CiA8L3JkZjpSREY+CjwveDp4bXBtZXRhPgogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgCjw/eHBhY2tldCBlbmQ9InciPz7GSkcQAAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAB3RJTUUH5wIEEzMyEOtsNAAAAvVJREFUeNrtmjtoVEEUhr+slxAkLhIkWIQlSCDaJIUprGKhku4UJvhECx8oaCqLIGJlIcE0QUUkYuOjEHxwSpEFRawEUSSQgBIkSEAJYSsJy2KRUZbNrLkb79zZzc7f7Z2dued8c+Zx5k4LKUlEWoE9wCDQD3QDW03xEjAHfATeAu9UdTkNu1pScLwHGAWOAdtiVvsJPAYmVfVrQwIQkQ7gOnAKiNbZTBG4B1xR1cWGASAi+4EHwPaEmvwOnFDVfNK2bnLg/HngEZBNsNktwPHe3t4fMzMz7+s2AkTkInBzjZ58BrwGps3vCOgEeoC9wAiQ+0cbo6p6q+4AiIgAz4FMFcevAg/Xmt1FJAKOmPmjy/KXEjCsqi/qBoCI5IAPQIel+AlwVlULNbaZBaaAQ5biAtCvqnP/a3smoQCYrOL8hKoertV5AFPnKDBhKc4Cd+oiAkRk0IzpSt1X1dMJRdgUcMZSNKSqL31HwGXLs8/AhQTn11HgU8x3pxcBItINfLGA3Jf0ml0l0krALlWd9RUBI5Y28i42LKr6Bnhlsf+gzyEwZHl22+HW3db2AS8ARCQDDFhCMu8QQN68o1wDxpbUI2BzWTr7d31W1SVX3pulccmyJGZ9AGivskFxrUKVzkgdQKvlWRqHGMsxbXEOIKqSv/sAEPkAkPEEoJSkH0kDKHkCENXLEEgDQLHZI4AwBOp4COAJQNNHQJgDnJ8HmHO/G8AOoM3kAZWHlr9Y+cTlUjnL1nfe5Ah/3n9JVb8lPY6fWrK/SrUBOz1EQFdZZwyYTtqddOj00TjqczF2ogYCELkAkGEDakM65QpAqdkBFBvIr6ILANMNBCC2rbXM7MPAXeJfc/GlReAcQUHJ5gK1SkTGWX1HaExVF9ao12lyjnItqOqY1x3TehhY8oJrMeq1Aycrns0CTgCEjVAAEAAEAAFAABAABAABQAAQAAQAAUAAEAAEAAFAABAABAABQDPJ5bH4PCtXZsoV56NlkdX3jOZdGfkb8Ju7NocaWkcAAAAASUVORK5CYII=\">"
    "\r\n</hread>\r\n<body>\r\n";

static String head(const StripSettings &stripSettings) {
  return String(HEAD_1) + stripSettings.STRIP_NAME + HEAD_2;
}

static String tail() {
  return "</body>\r\n</html>\r\n";
}


static void sendResult(const String &resp) {
  server.send(HTTP_OK, "text/html", resp);
}

static void sendJsonResult(const String resp) {
  Serial.println(String("StackSize in send json: ") + uxTaskGetStackHighWaterMark(NULL));
  server.send(HTTP_OK, "application/json", resp);
}

static String statusBody(const State &state,const Led &led, const Effects &effects) {
  String res(""); // NOLINT(cppcoreguidelines-init-variables)
  res += String("<p>Version ") + FIRMWARE_FLAVOUR + " " + FIRMWARE_VERSION + "</p>";
  if (state.riseStart != 0 || state.riseStop != 0) {
    res += "<p>Rise time: " + String(state.riseStart - state.now) + " &rArr; " + String(state.riseStop - state.now) + "</p>";
  }
  if (state.fallStart != 0 || state.fallStop != 0) {
    res += "<p>Fall time: " + String(state.fallStart - state.now) + " &rArr; " + String(state.fallStop - state.now) + "</p>";
  }
  res += "<p>Dyn Level: " + String(state.dynLevel) + "</p>";
  res += "<p>Dyn Factor: " + String(state.dynFactor) + "</p>";
  const Palette pal = effects.dynGradColor(0);
  res += "<p>DynR: " + String(pal.red);
  res += "   DynG: " + String(pal.green);
  res += "   DynB: " + String(pal.blue) + "</p>";
  res += "<p>Led1: " + String(led.getLed(1), HEX) + "</p>";
  return res;
}

static String numInput(const char *label, const char *name, long min, long max, int value) {
  return String("<label>") + label + R"(:</label><input name=")" + name + R"(" type="number" min=")" + String(min) + R"( " max=")" + String(max) + R"(" value=")" + String(value) + R"("></p>)";
}

static String formBody(const Settings &settings, int8_t strip) {
  String strp = (strip < 0 ? "" : String(strip)); // NOLINT(cppcoreguidelines-init-variables)
  String res(R"(<form action="/{{STRIP}}" method="post">)"); // NOLINT(cppcoreguidelines-init-variables)
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
  res += R"(<p><a href="/{{STRIP}}">Reload</a></p>\r\n)";
  res += R"(<form action="/setup" method="post">)";
  res += R"(<button type="submit">Reset WiFi</button>)";
  res += "</form>\r\n";
  res.replace("{{STRIP}}", strp);
  return res;
}

static bool extractArg8(const char *arg, uint8_t &target) {
  String str = server.arg(arg); // NOLINT(cppcoreguidelines-init-variables)
  if (str.length() > 0) {
    target = str.toInt();
    return true;
  }
  return false;
}

static bool extractArg16(const char *arg, uint16_t &target) {
  String str = server.arg(arg); // NOLINT(cppcoreguidelines-init-variables)
  if (str.length() > 0) {
    target = str.toInt();
    return true;
  }
  return false;
}

static bool extractArg32(const char *arg, uint32_t &target) {
  String str = server.arg(arg); // NOLINT(cppcoreguidelines-init-variables)
  if (str.length() > 0) {
    target = str.toInt();
    return true;
  }
  return false;
}

static void processSettings(const Settings &settings, State &state, bool wasOn) {
  time_t now = millis();
  if ((settings.on != 0) != wasOn) {
    if (settings.on != 0) {
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
  const bool wasOn = settings.on != 0;
  extractArg8("on", settings.on);
  extractArg8("mode", settings.mode);
  extractArg8("onoffmode", settings.onoffmode);
  extractArg8("colidx", settings.colidx);
  extractArg8("ngradient", settings.ngradient);
  extractArg16("bri", settings.bri);
  extractArg8("bri2", settings.bri2);
  extractArg32("cycle", settings.cycle);
  uint8_t bril = 0;
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
  sendResult(head(stripSettings) + formBody(settings, strip) + statusBody(state, led, effects) + tail());
}

static void handleSet(Settings &settings, State &state, const Led &led, const Effects &effects, const StripSettings &stripSettings, uint8_t strip, bool sendRes) {
  extractArgs(settings, state);
  if (sendRes) {
    sendResult(head(stripSettings) + formBody(settings, strip) + statusBody(state, led, effects) + "<p>sent command</p>\r\n" + tail());
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

  const Palette pal = effects.dynGradColor(0);
  jsState["dynR"] = pal.red;
  jsState["dynG"] = pal.green;
  jsState["dynB"] = pal.blue;

  for (const Palette palentry: settings.palette) {
    const JsonObject col = jsPalette.createNestedObject();
    col["r"] = palentry.red;
    col["g"] = palentry.green;
    col["b"] = palentry.blue;
  }


  String jsonString; // NOLINT(cppcoreguidelines-init-variables)
  serializeJson(jsonDocument, jsonString);
  // Serial.println(jsonString);
  sendJsonResult(jsonString);
}

static void extractSettings(Settings &settings, const JsonObject &jsSettings, const StripSettings &stripSettings) {
    if (jsSettings.containsKey("on")) {
      const bool isOn = static_cast<bool>(jsSettings["on"]);
      Serial.println(String("on: ") + isOn);
      settings.on = static_cast<uint8_t>(isOn);
    }
    if (jsSettings.containsKey("bri")) {
      settings.bri = jsSettings["bri"]; // NOLINT, false positive
      settings.bri %= NUM_IN_BYTE + 1;
    }
    if (jsSettings.containsKey("bri2")) {
      settings.bri2 = jsSettings["bri2"];
      if (settings.bri2 > stripSettings.MAX_BRI2) {
        settings.bri2 = stripSettings.MAX_BRI2;
      }
      settings.bri2 %= BRI2_MAXIMUM;
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
        const uint8_t idx = jsSetPal["idx"];
        if (idx < NUM_PALETTE) {
          const uint8_t red = jsSetPal["r"];
          const uint8_t green = jsSetPal["g"];
          const uint8_t blue = jsSetPal["b"];
          settings.palette[idx] = {red, green, blue};
        }
      }
    }
    if (jsSettings.containsKey("mode")) {
      settings.mode = jsSettings["mode"];
    }
}

static void handleApiPost(Settings &settings, State &state, const StripSettings &stripSettings, bool sendResult) {
  Serial.println(String("StackSize at start of post: ") + uxTaskGetStackHighWaterMark(NULL));
  const bool wasOn = settings.on != 0;
  DynamicJsonDocument jsonDocument(2536);
  String jsonString(server.arg("plain")); // NOLINT(cppcoreguidelines-init-variables)
  Serial.print("POST: ");
  Serial.println(jsonString);
  DeserializationError error = deserializeJson(jsonDocument, jsonString);
  String res = "OK"; // NOLINT(cppcoreguidelines-init-variables)
  if (error) {
    Serial.println(error.c_str());
    res = String("FAIL ") + error.c_str();
  } else {
    JsonObject root = jsonDocument.as<JsonObject>();
    JsonObject jsSettings = root["settings"];
    extractSettings(settings, jsSettings, stripSettings);
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
    const JsonObject col = jsLeds.createNestedObject();
    const InternalRgbw ledCol = led.getLedc(i);
    col["r"] = ledCol.red;
    col["g"] = ledCol.green;
    col["b"] = ledCol.blue;
    col["w"] = ledCol.white;
  }
  String jsonString; // NOLINT(cppcoreguidelines-init-variables)
  serializeJson(jsonDocument, jsonString);
  // Serial.println(jsonString);
  sendJsonResult(jsonString);
}

static String index() {
  String res(HTTP_MAIN); // NOLINT(cppcoreguidelines-init-variables)
  return res;
}

static void handleSpa(Settings &settings, State &state, const StripSettings &stripSettings, int8_t strip) {
  String strp = strip < 0 ? "" : String("/") + String(strip); // NOLINT(cppcoreguidelines-init-variables)
  extractArgs(settings, state);
  String indexData = index(); // NOLINT(cppcoreguidelines-init-variables)
  indexData.replace("{{SYSTEM_NAME}}", stripSettings.STRIP_NAME);
  indexData.replace("{{STRIP}}", strp);
  sendResult(indexData);
}

static void handleSetupRedir() {
  server.sendHeader("location", "/");
  server.send(HTTP_REDIRECT, "application/text", "redirect to root");
}

static void handleSetup() {
  handleSetupRedir();
  startWiFiPortal();
}


void initServer(Settings *settings, State *state, Led **led, Effects **effects) { // cppcheck-suppress unusedFunction
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
    String strip(i); // NOLINT(cppcoreguidelines-init-variables)
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
  Serial.print(String("Server Version ") + FIRMWARE_FLAVOUR + " " + FIRMWARE_VERSION + " started on ");
  Serial.println(WiFi.localIP());
}

void handleServer() { // cppcheck-suppress unusedFunction
  if (serverSetupDone) {
      server.handleClient();
  }
}
