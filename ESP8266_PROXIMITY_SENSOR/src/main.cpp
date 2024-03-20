#include <Arduino.h>

#define TRIGGER_PIN 12
#define ECHO_PIN 14
#define SOUND_VELOCITY 0.034 // define sound velocity in cm/uS
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

void setup(){
	Serial.begin(115200);	  // Starts the serial communication
	pinMode(TRIGGER_PIN, OUTPUT); // Sets the TRIGGER_PIN as an Output
	pinMode(ECHO_PIN, INPUT);  // Sets the ECHO_PIN as an Input
}

void loop(){
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

	// Convert to inches
	distanceInch = distanceCm * CM_TO_INCH;

	// Prints the distance on the Serial Monitor
	Serial.print("Distance (cm): ");
	Serial.println(distanceCm);
	Serial.print("Distance (inch): ");
	Serial.println(distanceInch);

	delay(1000);
}