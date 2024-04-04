#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char *ssid = "raspberrypi_5";
const char *password = "12345678";
// const char *MQTT_username = "vsh_pi5";
// const char *MQTT_password = "12345678";

// MQTT Broker
const char *mqtt_server = "10.42.0.1";
const int mqtt_port = 1883;

// MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);

// Topic to which ESP8266 will publish
const char *channel = "/esp/obstacle";

// Ultrasonic sensor
#define TRIGGER_PIN 12
#define ECHO_PIN 14
#define SOUND_VELOCITY 0.034 // define sound velocity in cm/uS
#define CM_TO_INCH 0.393701

#define UNIT_LOCATION 0 // up or down
#define UNIT_POSITION 0 // left or right

long duration;
float distanceCm;
float distanceInch;

bool thresh_flag;

// Function to reconnect to MQTT broker
void reconnect(){
	// Loop until we're reconnected
	while (!client.connected()){
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect("ESP8266Client")){
			Serial.println("connected");
		}
		else{
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

void setup(){
	pinMode(16, OUTPUT);
	pinMode(5, OUTPUT);
	Serial.begin(115200); // Starts the serial communication
	// Connect to Wi-Fi
	WiFi.begin(ssid, password);
	Serial.println();

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED){
		delay(500);
		Serial.print("Wifi not connected! Retrying...");
		digitalWrite(5, HIGH);
	}

	digitalWrite(5, LOW);

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	// Setup MQTT client
	client.setServer(mqtt_server, mqtt_port);

	// Setup ultrasonic sensor
	pinMode(TRIGGER_PIN, OUTPUT); // Sets the TRIGGER_PIN as an Output
	pinMode(ECHO_PIN, INPUT);	  // Sets the ECHO_PIN as an Input
	

	thresh_flag = false;
}

void loop()
{
	if (!client.connected()){
		reconnect();
	}

	digitalWrite(16, HIGH);
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

	// Polling to check threshold
	bool threshold = false;

	if (distanceCm < 15) threshold = true; // Send interrupt if distance is less than 3inch
	else threshold = false;

	// Generate payload
	String payload="Stop right there. Back off.";

	// payload = "{\"th\":" + String(threshold) + ",\"loc\":" + String(UNIT_LOCATION) + ",\"pos\":" + String(UNIT_POSITION) + ",\"msg\":\"Stop right there\"}";

	// Send data
	if (threshold && !thresh_flag){
		client.publish(channel, payload.c_str());
		thresh_flag = true;
	}
	else{
		thresh_flag = false;
	}
	
	delay(1000);
}
