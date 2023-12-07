// Arduino9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Arduino9x_TX

#include <SPI.h>
#include <RH_RF95.h>

#include "log_lib.h"
#include "config.h"

#define RFM95_CS 7
#define RFM95_RST 3
#define RFM95_INT 4

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 868.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Blinky on receipt
#define LED LED_BUILTIN

MessageLogger info_logger;

void setup() 
{
  pinMode(LED, OUTPUT);     
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  digitalWrite(LED, HIGH);
	
	Serial.begin(9600);
	unsigned long saved_time = millis();
	while (!Serial) 	//this empty while is intentional, sometimes serial connection is not established immediately, but we need it so we wait...
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
			}

			created = true;	
			Serial.println("Created new folder as: " + log_folder_name);
		}
    	else //else we try again with log_(n+1).txt
			++n;
	}

	info_logger = MessageLogger(log_folder_name + "/info", "message");

  info_logger.record_event("Battery status is: " + String(batteryStatus()) + " volts");
  info_logger.record_event("Arduino LoRa RX Test!");
  
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    info_logger.record_event("LoRa radio init failed");
    while(42);
  }
  info_logger.record_event("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    info_logger.record_event("setFrequency failed");
    while(42);
  }
  info_logger.record_event("Set Freq to: " + String(RF95_FREQ));

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

}

void loop()
{
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf, &len))
    {
      digitalWrite(LED, LOW);
      //RH_RF95::printBuffer("Received: ", buf, len);
      //Serial.print("Got: ");
      //Serial.println((char*)buf);
      //Serial.print("RSSI: ");
      info_logger.record_event(String(rf95.lastRssi()) + " " + String(rf95.lastSNR()));
      
      // Send a reply
      uint8_t data[] = "And hello back to you";
	  delay(10);
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      //Serial.println("Sent a reply");
      digitalWrite(LED, HIGH);
    }
    else
    {
      info_logger.record_event("Receive failed");
    }
  }
}