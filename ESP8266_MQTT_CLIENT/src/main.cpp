#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "OPPOA57";
const char* password = "11111111";
const char* MQTT_username = "rushikesh"; 
const char* MQTT_password = "rushi2004"; 

// MQTT Broker
const char* mqtt_server = "rushikesh.cloud.shiftr.io";
const int mqtt_port = 1883; // Default MQTT port

// MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);

// Topic to which ESP8266 will publish
const char* topic = "esp8266/data";

// Function to reconnect to MQTT broker
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client","rushikesh","rushi2004")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println();

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup MQTT client
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  
  // Send data
  float temperature = 25.5; // Example temperature data
  String payload = String(temperature);
  client.publish(topic, payload.c_str());

  delay(5000); // Publish data every 5 seconds
}
