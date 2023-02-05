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
    String jsonString;
    serializeJson(json, jsonString);
    publish(topic, jsonString);
}


static void subscribe(const String &topic) {
    pubSubClient.subscribe(topic.c_str());
}

static void publishOneDiscovery(int8_t stripNo) {
    DynamicJsonDocument jsonDocument(1024);
    JsonObject root = jsonDocument.to<JsonObject>();
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
    JsonObject device = root.createNestedObject("device");  
    JsonArray identifiers = device.createNestedArray("identifiers");
    identifiers.add("device-" + wiFiMac);
    device["name"] = String(SYSTEM_NAME) + " Device";
    device["model"] = "ESP Light Strip";
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
            publishOneDiscovery(i);
        }
    }
}

static String getJsonState(int8_t stripNo) {
    DynamicJsonDocument jsonDocument(1024);
    JsonObject root = jsonDocument.to<JsonObject>();
    if (stripNo < 0) {
        stripNo = 0;
    }
    root["state"] = strip_settings[stripNo].on ? "ON" : "OFF";
    root["brightness"] = strip_settings[stripNo].bri;
    String state;;
    serializeJson(jsonDocument, state);
    return state;
}

static void publishState() {
    static String lastJsonState[NUM_STRIPS + 1];
    for (int8_t i = -1; i < NUM_STRIPS; i++) {
        String state = getJsonState(i);
        if (lastJsonState[i+1] != state) {
            String topic = stateTopic(i);
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

void handleMqttMessage(char* p_topic, byte* p_payload, unsigned int p_length) {
  // concatenates the payload into a string
  String payload;
  for (uint8_t i = 0; i < p_length; i++) {
    payload.concat((char)p_payload[i]);
  }
  Serial.println(String("Command: ") + p_topic + " " + payload);
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

void initMqtt() {
  Serial.print("Resetting WIFI");
  pubSubClient.setBufferSize(2048);
  pubSubClient.setServer(mqtt_server, 1883);
  pubSubClient.setCallback(handleMqttMessage);
}

void handleMqtt() {
    static unsigned long lastConnectAttempt = 0;
    static unsigned long lastSent = 0;
    static uint16_t count = 0;
    unsigned long now = millis();

    if (!pubSubClient.connected()) {
        if (now - lastConnectAttempt > 5000) {
            reconnect();
            lastConnectAttempt = millis();
        }
    } else {
        pubSubClient.loop();
        publishState();
        if (now - lastSent > 1000) {
            publishAvailability();
            // pubSubClient.publish("mqtttest/count", String(count).c_str());
            Serial.print("#");
            lastSent = millis();
        }
    }
    count++;
}