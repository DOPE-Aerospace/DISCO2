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
static char inputbuffer[18] = {}; // Max possible lenght of LTM packet

//Logger Globals
MessageLogger info_logger;
Logger flight_logger;

//Adding a new timer is simple, add it before the last enum.
enum timer
{
	NUMBER_OF_JOBS //THIS HAS TO BE THE LAST ENUM
};

unsigned long saved_times[NUMBER_OF_JOBS] = {};

template<typename F>
void if_time_expired(timer job, unsigned long delay, F fn)
{
	if (millis() - saved_times[job] > delay)
  {
		fn();
		saved_times[job] = millis();
	}
}

int lenght_payload(char function_byte){
	switch(function_byte){
		case 'G':
			return 14;
		case 'A':
			return 6;
		case 'S':
			return 7;
		case 'O':
			return 14;
		case 'N':
			return 6;
		case 'X':
			return 6;
		case 'T': // This type of packet is not used by INAV, but it's still part of LTM.
			return 12;
		default:
			return -1;
	}
}

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
	flight_logger = Logger(log_folder_name + "/flight", "LTM");

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
	inputbuffer[0] = '$';
	inputbuffer[1] = 'T';
	
	//===========
	//   Misc
	//===========
	info_logger.record_event("Battery status is: " + String(batteryStatus()) + " volts");
	info_logger.record_event("Setup finished.");

}

void loop()
{

	if(Serial1.find("$T")){
		digitalWrite(LED_BUILTIN, LOW);

		Serial1.readBytes(inputbuffer + 2, 1);
		int packet_lenght = lenght_payload(inputbuffer[2]);

		if(packet_lenght != -1){

			packet_lenght += 1;
			size_t read = Serial1.readBytes(inputbuffer + 3, packet_lenght);

			packet_lenght += 3;
			rf95.send(reinterpret_cast<const uint8_t*>(inputbuffer), packet_lenght);
			flight_logger.record_event(reinterpret_cast<const uint8_t*>(inputbuffer), packet_lenght);

		}

		if(Serial1.available() > 10){
			Serial.println(Serial1.available());
		}
		digitalWrite(LED_BUILTIN, HIGH);
	}

}