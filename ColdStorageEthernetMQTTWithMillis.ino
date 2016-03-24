/* ESP8266 + MQTT Garage Door State and Temperature Node
   Can also receive commands; adjust messageReceived() function
   Modified from a sketch from MakeUseOf.com
   Written by: Loral Godfrey, Jan 16, 2016
*/

#include <Ethernet.h>
#include <MQTTClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192,168,1,101 };

// MQTT
const char* ssid = "ssid";
const char* password = "password";

String subscribeTopic = "openhab/coldStorage/arduino"; // subscribe to this topic; anything sent here will be passed into the messageReceived function
String tempTopic = "openhab/coldStorage/temperature"; //topic to publish temperatures readings to
const char* server = "192.168.1.22"; // server or URL of MQTT broker
String clientName = "coldStorageArduino"; // just a name used to talk to MQTT broker

EthernetClient ethernetClient;
MQTTClient mqttClient;

String tPayload;

// Temperatrue
#define ONE_WIRE_BUS 7 // Data wire is plugged into pin 12 on the ESP8266
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float t;
unsigned long previousMillis = 0;
const long getTempInterval = 60000; // in milliseconds
unsigned long currentMillis = millis();

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  mqttClient.begin(server, ethernetClient);
  delay(1000);
  Serial.print("connecting...");
  if (ethernetClient.connect(server, 80)) {
    Serial.println("connected");
    ethernetClient.println("GET /search?q=arduino HTTP/1.0");
    ethernetClient.println();
  } else {
    Serial.println("connection failed");
  }

  Serial.print("Connecting to ");
  Serial.print(server);
  Serial.print(" as ");
  Serial.println(clientName);

  if (mqttClient.connect((char*) clientName.c_str())) {
    Serial.println("Connected to MQTT broker");
    Serial.print("Subscribed to: ");
    Serial.println(subscribeTopic);
    mqttClient.subscribe(subscribeTopic);
  }
  else {
    Serial.println("MQTT connect failed");
    Serial.println("Will reset and try again...");
    abort();
  }

  // Dallas sensor startup
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
}

void loop() {
  currentMillis = millis();
  if (currentMillis - previousMillis >= getTempInterval)
  {
    previousMillis = currentMillis;
    sensors.requestTemperatures(); // Send the command to get temperatures
    //t = sensors.getTempCByIndex(0);
    t = sensors.getTempFByIndex(0);
    if (isnan(t) || !mqttClient.connected()) {}
    else {
      //tPayload = String((t * 1.8 + 32), 2); // convert C to F
      tPayload = String(t,2);
      Serial.println(tPayload);
      mqttClient.publish(tempTopic, tPayload);
      Serial.println("published temperatrue");
    }
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}
