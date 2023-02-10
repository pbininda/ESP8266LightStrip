#include <stdint.h>
#include <HardwareSerial.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "WiFiServe.h"
#include "state.h"

WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);

static String stripId(int8_t stripNo) {
    if (stripNo < 0) {
        return wiFiMac;
    }
    return wiFiMac + "-" + String(stripNo);
}

static String stateTopic(int8_t stripNo) {
    return  "esp/" + stripId(stripNo) + "/light/state";
}

static String availabilityTopic(int8_t stripNo) {
    return  "esp/" + stripId(stripNo) + "/light/availability";
}

static String commandTopic(int8_t stripNo) {
    return  "esp/" + stripId(stripNo) + "/light/set";
}

static String discoveryTopic(int8_t stripNo) {
    return String("homeassistant/light/") + stripId(stripNo) + "/config";
}

static void publish(const String &topic, const String &payload) {
    pubSubClient.publish(topic.c_str(), payload.c_str(), true);
}

static void publish(const String &topic, const JsonDocument &json) {
    String jsonString; // NOLINT(cppcoreguidelines-init-variables)
    serializeJson(json, jsonString);
    publish(topic, jsonString);
}


static void subscribe(const String &topic) {
    pubSubClient.subscribe(topic.c_str());
}

static void publishOneDiscovery(int8_t stripNo) {
    DynamicJsonDocument jsonDocument(1024);
    const JsonObject root = jsonDocument.to<JsonObject>();
    if (stripNo < 0) {
        root["name"] = String(SYSTEM_NAME) + " Global";
    } else {
        root["name"] = STRIP_SETTINGS[stripNo].STRIP_NAME;
    }
    root["unique_id"] = "entity-" + wiFiMac + (stripNo < 0 ? "" : (String("-") + String(stripNo)));
    root["schema"] = "json";
    root["availability_topic"] = availabilityTopic(stripNo);
    root["state_topic"] = stateTopic(stripNo);
    root["command_topic"] = commandTopic(stripNo);
    root["brightness"] = true;
    root["effect"] = true;
    JsonArray effects = root.createNestedArray("effect_list");
    effects.add("fixed");
    effects.add("zylon");
    effects.add("wheel");
    JsonObject device = root.createNestedObject("device");  
    JsonArray identifiers = device.createNestedArray("identifiers");
    identifiers.add("device-" + wiFiMac);
    device["name"] = String(SYSTEM_NAME) + " Device";
    device["model"] = "ESP Light Strip";
    device["manufacturer"] = "PBininda";
    device["sw_version"] = SW_VERSION_NO;
    // root["rgb"] = true;
    publish(discoveryTopic(stripNo), jsonDocument);
}

static void publishDiscovery(bool clear) {
    if (clear) {
        publish(discoveryTopic(-1), "");
        for (uint8_t i = 0; i < NUM_STRIPS; i++) {
            publish(discoveryTopic(i), "");
        }
    } else {
        publishOneDiscovery(-1);
        for (uint8_t i = 0; i < NUM_STRIPS; i++) {
            publishOneDiscovery(static_cast<int8_t>(i));
        }
    }
}

static String getJsonState(int8_t stripNo) {
    DynamicJsonDocument jsonDocument(1024);
    const JsonObject root = jsonDocument.to<JsonObject>();
    if (stripNo < 0) {
        stripNo = 0;
    }
    root["state"] = strip_settings[stripNo].on != 0 ? "ON" : "OFF";
    root["brightness"] = strip_settings[stripNo].bri;
    root["effect"] = strip_settings[stripNo].mode == 2 ? "wheel" : strip_settings[stripNo].mode == 1 ? "zylon" : "fixed";
    String state;; // NOLINT(cppcoreguidelines-init-variables)
    serializeJson(jsonDocument, state);
    return state;
}

static void publishState() {
    static String lastJsonState[NUM_STRIPS + 1];
    for (int8_t i = -1; i < NUM_STRIPS; i++) {
        const String state = getJsonState(i); // NOLINT(cppcoreguidelines-init-variables)
        if (lastJsonState[i+1] != state) {
            String topic = stateTopic(i); // NOLINT(cppcoreguidelines-init-variables)
            Serial.println("state changed: " + String(i) + " " + topic + " " +state);
            publish(topic, state);
            lastJsonState[i+1] = state;
        }
    }
}

static void publishAvailability() {
    for (int8_t i = -1; i < NUM_STRIPS; i++) {
     publish(availabilityTopic(i), "online");
   }
}

void applyPayloadToStrip(uint8_t stripNo, JsonObject &root) {
    if (root.containsKey("state")) {
        const String state = root["state"]; // NOLINT(cppcoreguidelines-init-variables)
        if (state == "ON") {
            strip_settings[stripNo].on = 1;
        } else {
            strip_settings[stripNo].on = 0;
        }
    }
    if (root.containsKey("brightness")) {
        const uint8_t brightness = root["brightness"];
        strip_settings[stripNo].bri = brightness;
    }
    if (root.containsKey("effect")) {
        const String effect = root["effect"]; // NOLINT(cppcoreguidelines-init-variables)
        if (effect == "fixed") {
            strip_settings[stripNo].mode = 0;
        } else if (effect == "zylon") {
            strip_settings[stripNo].mode = 1;
        } else if (effect == "wheel") {
            strip_settings[stripNo].mode = 2;
        }
    }
}

void handleMqttMessage(char* p_topic, byte* p_payload, unsigned int p_length) {
    // concatenates the payload into a string
    String payload; // NOLINT(cppcoreguidelines-init-variables)
    char *savePtrTopic = nullptr;
    char *savePtrId = nullptr;
    for (unsigned int i = 0; i < p_length; i++) {
        payload.concat(static_cast<char>(p_payload[i]));
    }
    const char * top1 = strtok_r(p_topic, "/", &savePtrTopic);
    if (top1 == nullptr) {
        return;
    }
    char * stripId = strtok_r(nullptr, "/", &savePtrTopic);
    if (stripId == nullptr) {
        return;
    }
    const char * light = strtok_r(nullptr, "/", &savePtrTopic);
    const char * cmd = strtok_r(nullptr, "/", &savePtrTopic);
    const char * id1  = strtok_r(stripId, "-", &savePtrId);
    if (id1 == nullptr) {
        return;
    }
    const char * id2 = strtok_r(nullptr, "-", &savePtrId);
    int8_t stripNo(-1);
    if (id2 != nullptr) {
        stripNo = strtol(id2, nullptr, DEC);
    } else {
        id2 = "-1";
        stripNo = -1;
    }
    Serial.println(String("Command: ") + top1 + " " + id1 + " " + String(stripNo) + " " + light + " " + cmd + " " + payload);
    if (stripNo < -1 || stripNo >= NUM_STRIPS) {
        return;
    }
    DynamicJsonDocument jsonDocument(1024);
    DeserializationError error = deserializeJson(jsonDocument, payload);
    if (error) {
        Serial.println(error.c_str());
    } else {
        JsonObject root = jsonDocument.as<JsonObject>();
        if (stripNo < 0) {
            for (uint8_t i = 0; i < NUM_STRIPS; i++) {
                applyPayloadToStrip(i, root);
            }
        } else {
            applyPayloadToStrip(stripNo, root);
        }
    }
}

 

static void reconnect() {
  if (!pubSubClient.connected()) {
    Serial.print(String("Attempting MQTT connection...") + pubSubClient.state() + "...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (pubSubClient.connect("ESP32 cleint", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      publishDiscovery(false);
      for (int8_t i = -1; i < NUM_STRIPS; i++) {
          subscribe(commandTopic(i));
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(pubSubClient.state());
    }
  }
}

static const uint32_t MQTT_BUFFER_SIZE = 2048;
static const uint32_t MQTT_SERVER_PORT = 1883;

void initMqtt() {
  Serial.print("Resetting WIFI");
  pubSubClient.setBufferSize(MQTT_BUFFER_SIZE);
  pubSubClient.setServer(mqtt_server, MQTT_SERVER_PORT);
  pubSubClient.setCallback(handleMqttMessage);
}

static const uint32_t CONNECT_INTERVAL_MS = 5000;
static const uint32_t PUBLISH_INTERVAL_MS = 1000;

void handleMqtt() {
    static uint16_t count = 0;
    uint32_t now = millis();

    if (!pubSubClient.connected()) {
        static uint32_t lastConnectAttempt = 0;
        if (now - lastConnectAttempt > CONNECT_INTERVAL_MS) {
            reconnect();
            lastConnectAttempt = millis();
        }
    } else {
        static uint32_t lastSent = 0;
        pubSubClient.loop();
        publishState();
        if (now - lastSent > PUBLISH_INTERVAL_MS) {
            publishAvailability();
            // pubSubClient.publish("mqtttest/count", String(count).c_str());
            Serial.print("#");
            lastSent = millis();
        }
    }
    count++;
}