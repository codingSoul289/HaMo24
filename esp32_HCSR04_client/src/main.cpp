#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "raspberrypi_4b"; //raspberrypi_4b
const char* password = "raspi_4b"; //raspi_4b
const char* MQTT_username = "vsh_pi4b"; //vsh_pi4b
const char* MQTT_password = "12345678"; //12345678

// MQTT Broker
const char* mqtt_server = "10.42.0.1"; //10.42.0.1
const int mqtt_port = 1883; // Default MQTT port

// MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);

// Topic to which ESP8266 will publish
const char* channel = "/esp/obstacle";

// Ultrasonic sensor
#define TRIGGER_PIN 5
#define ECHO_PIN 18
#define SOUND_VELOCITY 0.034 // define sound velocity in cm/uS
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

// Function to reconnect to MQTT broker
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client",MQTT_username,MQTT_password)) {
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
  Serial.begin(115200); // Starts the serial communication
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println();

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Wifi not connected! Retrying...");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup MQTT client
  client.setServer(mqtt_server, mqtt_port);

  // Setup ultrasonic sensor
  pinMode(TRIGGER_PIN, OUTPUT); // Sets the TRIGGER_PIN as an Output
	pinMode(ECHO_PIN, INPUT);  // Sets the ECHO_PIN as an Input
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  
  // Sensing distance using ultrasonic sensor
  // Clears the TRIGGER_PIN
	digitalWrite(TRIGGER_PIN, LOW);
	delayMicroseconds(2);
	
	// Sets the TRIGGER_PIN on HIGH state for 10 micro seconds
	digitalWrite(TRIGGER_PIN, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIGGER_PIN, LOW);

	// Read the ECHO_PIN, returns the sound wave travel time in microseconds
	duration = pulseIn(ECHO_PIN, HIGH);

	// Calculate the distance
	distanceCm = duration * SOUND_VELOCITY / 2;

	// Prints the distance on the Serial Monitor
	Serial.print("Distance (cm): ");
	Serial.println(distanceCm);

  // Check threshold
  bool threshold = false;
  if(distanceCm < 15)
  threshold = true; // Send true if distance is less than 2 cm

  // Send data
  String payload = String(threshold);
  client.publish(channel, payload.c_str());

  delay(1000); // Publish data every second
}
