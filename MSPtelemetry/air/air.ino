#include <SPI.h>
#include <RH_RF95.h>

//#include <TinyGPS++.h>
//#include <MPU6050_light.h>
//#include <Adafruit_Sensor.h>
//#include "Adafruit_BMP3XX.h"

#include "log_lib.h"
#include "config.h"

#define RFM95_CS 7
#define RFM95_RST 3
#define RFM95_INT 4

// Frequency
#define RF95_FREQ 868.0
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Blink on receipt
#define LED LED_BUILTIN

//Flight Globals
static const unsigned long FlightBaud = 2400;
static char requestbuffer[50] = {}; // 50 is just for testing, a MSP packet may be longer
static char responsebuffer[50] = {}; // 50 is just for testing, a MSP packet may be longer

//Logger Globals
MessageLogger info_logger;

void setup() 
{
	pinMode(LED, OUTPUT);
	digitalWrite(LED, HIGH);
	
	Serial.begin(9600);
	unsigned long saved_time = millis();
	while (!Serial)
	{
		if (saved_time+1000u < millis()) //after onesecond 
    	break; 
	}

	//=============
	// Logger Init
	//=============
	unsigned int n = 0;  //counter for file creation
	bool created = false;
	String log_folder_name;

	while (!created) //if this log file already exists, we create another in the format log_2.txt
	{       
		log_folder_name = "yeet_" + String(n);

		if (!file_exists(log_folder_name)) //if the file is NOT present on the SD
    	{  

			Serial.println(log_folder_name);

			if (!make_dir(log_folder_name)) 
    		{
				Serial.println(F("ERROR_FILE3: Can't create directory."));
				abort_blink(1);
			}

			created = true;	
			Serial.println("Created new folder as: " + log_folder_name);
		}
    	else //else we try again with log_(n+1).txt
		{
			++n;
		}
	}

	info_logger = MessageLogger(log_folder_name + "/info", "message");

	//=======
	// ANTENNA
	//=======
	
	info_logger.record_event("Arduino LoRa RX Test!");

	// manual reset
	pinMode(RFM95_RST, OUTPUT);
	digitalWrite(RFM95_RST, HIGH);
	digitalWrite(RFM95_RST, LOW);
	delay(10);
	digitalWrite(RFM95_RST, HIGH);
	delay(10);

	if(!rf95.init()) {
		info_logger.record_event("LoRa radio init failed");
		abort_blink(2);
	}
	info_logger.record_event("LoRa radio init OK!");

	// Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
	if(!rf95.setFrequency(RF95_FREQ)){
		info_logger.record_event("setFrequency failed");
		abort_blink(3);
	}
	info_logger.record_event("Set Freq to: " + String(RF95_FREQ));

	// Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

	// The default transmitter power is 13dBm, using PA_BOOST.
	// If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
	// you can set transmitter powers from 2 to 20 dBm:
	rf95.setTxPower(20, false);

	//=======
	// FLIGHT
	//=======
	info_logger.record_event("Connecting flight controller serial...");
	Serial1.begin(FlightBaud);
	saved_time = millis();
	while (!Serial1)
	{
		if (saved_time+1000u < millis()){ //after onesecond 
			info_logger.record_event("Flight controller connection failed");
			abort_blink(4);
		}
	}
	responsebuffer[0] = '$';
	responsebuffer[1] = 'X';
	
	//===========
	//   Misc
	//===========
	info_logger.record_event("Battery status is: " + String(batteryStatus()) + " volts");
	info_logger.record_event("Setup finished.");

}

void loop()
{

	rf95.waitAvailable();
	digitalWrite(LED, LOW);

	uint8_t received = sizeof(requestbuffer);
	rf95.recv(reinterpret_cast<uint8_t*>(requestbuffer), &received);

	Serial1.write(requestbuffer, received);

	if(Serial1.find("$X")){

		Serial1.readBytes(responsebuffer + 2, 6);
		int payload_lenght = responsebuffer[6] + (responsebuffer[7] * 256);

		size_t read = Serial1.readBytes(responsebuffer + 8, payload_lenght + 1); //+1 per il controllo errori

		rf95.send(reinterpret_cast<const uint8_t*>(responsebuffer), 8 + payload_lenght + 1);

		digitalWrite(LED, HIGH);
	}

}