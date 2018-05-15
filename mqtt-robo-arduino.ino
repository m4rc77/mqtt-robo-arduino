// Board Libraries
#include <ESP8266WiFi.h>

// Required libraries:
#include <Servo.h>
#include <PubSubClient.h>

// The Wemos D1 mini's pins do not map to arduino pin numbers accurately ... see
// https://github.com/esp8266/Arduino/issues/1243
#define S1_PIN 5 // => D1
#define S2_PIN 4 // => D2

#include "config.h"
// File config.h sould contain the configuration in the following form ...
/*
const char *ssid       = "your WLAN SSID";
const char *password   = "your WLAN password";
const char *mqttServer = "the broker url";

const char *mqttTopicLeft  = "topic-for-robo-left";
const char *mqttTopicRight = "topic-for-robo-rightright";
*/

// Variables ...
Servo servo1;  // create servo object to control a servo
Servo servo2;  // create servo object to control a servo

WiFiClient   espClient;
PubSubClient mqttClient(espClient);

void setup() {
  Serial.begin(9600);

  // setup io-pins
  pinMode(S1_PIN, OUTPUT);
  pinMode(S2_PIN, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);  // initialize onboard LED as output

  setupServo();
  setupWifi();
  setupMqtt();
  flashLed();
}

void setupServo() {
  servo1.attach(S1_PIN);
  servo2.attach(S2_PIN);
  servo1.write(70);
  servo2.write(70);
}

void setupWifi() {
  delay(10);
  Serial.println();Serial.print("Connecting to ");Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");Serial.println(WiFi.localIP());
}

void setupMqtt() {
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(mqttCallback);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  Serial.print("Message arrived [");Serial.print(topicStr);Serial.print("]='");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.print("', length=");Serial.println(length);
  
  String s = String((char*)payload);
  String sub = s.substring(0, length); // no idea why ... but the conversion above does not work and 0ht is added to the end ...
  Serial.println(sub);
  int servoPos = sub.toInt();
  
  if (topicStr == mqttTopicLeft) {
    Serial.print("Got left command servo=");Serial.print(servoPos);Serial.println(" ...");
    servo1.write(servoPos);
  } else if (topicStr == mqttTopicRight) {
    Serial.print("Got left command servo=");Serial.print(servoPos);Serial.println(" ...");
    servo2.write(servoPos);
  }
}

void loop() {

  checkWifi();
  checkMqtt();
  mqttClient.loop();
}

void checkWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    setupWifi();  
  }
}

void checkMqtt() {
  if (!mqttClient.connected()) {
    while (!mqttClient.connected()) {
      Serial.print("Attempting to open MQTT connection...");
      // connect with last will (QoS=1, retain=true, ) ...
      if (mqttClient.connect("ESP8266_Client")) {
        Serial.println("connected");
        mqttClient.subscribe(mqttTopicLeft);
        mqttClient.subscribe(mqttTopicRight);
      } else {
        Serial.print("MQTT connection failed, retry count: ");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
      }
    }
  }  
}



// =================================================================================================================================
// Helper methods 
// =================================================================================================================================

void ledOn() {
  digitalWrite(BUILTIN_LED, HIGH);
}

void ledOff() {
  digitalWrite(BUILTIN_LED, LOW);
}

void flashLed() {
  for (int i=0; i < 5; i++){
      ledOn();
      delay(100);
      ledOff();
      delay(100);
   }  
   ledOn();
}  
