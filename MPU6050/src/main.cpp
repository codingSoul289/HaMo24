#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define LOOP_DELAY 0.004 // LOOP DELAY IN SECONDS
#define LOOP_DELAY_MS 4  // LOOP DELAY IN MS

#define I2C_SDA 21 		  // GPIO 21
#define I2C_SCL 22 		  // GPIO 22
#define MPU6050_ADDR 0x68 // MPU6050 SLAVE ADDRESS

#define YAW_RATE_THRESHOLD 5 // gyroZ values below threshold are rejected for yaw calculation to reduce noise effects
#define ACC_TO_VEL_THRESHOLD 0.9 // threshold on acceleration when calculating velocity

TwoWire ports(1); // two wire variable initialised with i2c bus_num = 1
Adafruit_MPU6050 mpu;

int loop_count = 0; // used to slow down the data display rate

// kalman variables
double kalman_angX=0, kalman_uncertainty_angX = 2*2;
double kalman_angY=0, kalman_uncertainty_angY = 2*2;
double kalman_angZ=0, kalman_uncertainty_angZ = 2*2;

// measurement variables
double angX;
double angY;
double angZ;

double ax_calib=0;
double ay_calib=0;
double az_calib=0;
double gx_calib=0;
double gy_calib=0;
double gz_calib=0;

void kalman_filter(double *kalman_state, double *kalman_uncertainty, double kalman_input, double kalman_measurement){
	*kalman_state += (LOOP_DELAY) * kalman_input;
	*kalman_uncertainty += LOOP_DELAY * LOOP_DELAY * 4 * 4;
	double kalman_gain = *kalman_uncertainty / (*kalman_uncertainty + 3*3);

	*kalman_state += kalman_gain * (kalman_measurement - *kalman_state);
	*kalman_uncertainty *= (1-kalman_gain);
}

void setup(){
	// set SDA and SCL pins
	ports.setPins(I2C_SDA, I2C_SCL);

	Serial.begin(9600);
	while(!Serial) delay(10);

	Serial.println("MPU6050 TEST");

	while(!mpu.begin(MPU6050_ADDR, &ports)){
		Serial.println("Failed to find MPU6050");
		delay(10);
	}

	Serial.println("Found MPU6050");

	for(int i=0;i<2000;i++){
		sensors_event_t a, g, temp;
    	mpu.getEvent(&a, &g, &temp);

		ax_calib += a.acceleration.x;
		ay_calib += a.acceleration.y;
		az_calib += a.acceleration.z;
		gx_calib += g.gyro.x;
		gy_calib += g.gyro.y;
		gz_calib += g.gyro.z;
	}

	ax_calib/=2000;
	ay_calib/=2000;
	az_calib/=2000;
	az_calib-=9.8066;
	gx_calib/=2000;
	gy_calib/=2000;
	gz_calib/=2000;

	Serial.println("Setup Complete...");
}

void loop(){
    // Get new sensor data
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

	double ax,ay,az,gx,gy,gz;
	ax = a.acceleration.x - ax_calib;
	ay = a.acceleration.y - ay_calib;
	az = a.acceleration.z - az_calib;

	gx = (g.gyro.x - gx_calib) * RAD_TO_DEG;
	gy = (g.gyro.y - gy_calib) * RAD_TO_DEG;
	gz = (g.gyro.z - gz_calib) * RAD_TO_DEG;

	angX = atan(ay / sqrt(ax*ax + az*az)) * RAD_TO_DEG;
	angY = (-1) * atan(ax / sqrt(ay*ay + az*az)) * RAD_TO_DEG;
	angZ = kalman_angZ + (abs(gz) > YAW_RATE_THRESHOLD) * gz * LOOP_DELAY;

	kalman_filter(&kalman_angX, &kalman_uncertainty_angX, gx, angX);

	kalman_filter(&kalman_angY, &kalman_uncertainty_angY, gy, angY);

	kalman_filter(&kalman_angZ, &kalman_uncertainty_angZ, gz, angZ);

	if(loop_count == 0){

		Serial.print("angX : ");
		Serial.print(kalman_angX);
		Serial.println("");

		Serial.print("angY : ");
		Serial.print(kalman_angY);
		Serial.println("");

		Serial.print("angZ : ");
		Serial.print(kalman_angZ * 1.5);
		Serial.println("");
		
		Serial.println("--------------------------------------");
	}

	if(loop_count < 250) loop_count++;
	else loop_count = 0;
	

	delay(LOOP_DELAY_MS);
}