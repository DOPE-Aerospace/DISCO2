#include <SPI.h>
#include <RH_RF95.h>

#include "status_handler.h"
#include "MSP_forwarder.h"

#define RFM95_CS 4
#define RFM95_RST 2
#define RFM95_INT 3

// Frequency
#define RF95_FREQ 868.0
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Blink on receipt
#define LED LED_BUILTIN

void setup()
{
	pinMode(LED, OUTPUT);
	digitalWrite(LED, HIGH);

	Serial.begin(115200);
	unsigned long saved_time = millis();
	while(!Serial)
	{
		if(saved_time + 1000u < millis()) //after one second
		{
			abort_blink(1);
		}
	}

	//=======
	// ANTENNA
	//=======

	// manual reset
	pinMode(RFM95_RST, OUTPUT);
	digitalWrite(RFM95_RST, HIGH);
	digitalWrite(RFM95_RST, LOW);
	delay(10);
	digitalWrite(RFM95_RST, HIGH);
	delay(10);

	if(!rf95.init()){
		abort_blink(2);
	}

	// Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
	if(!rf95.setFrequency(RF95_FREQ)){
		abort_blink(3);
	}

	// Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

	// The default transmitter power is 13dBm, using PA_BOOST.
	// If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
	// you can set transmitter powers from 2 to 20 dBm:
	rf95.setTxPower(20, false);

}

void loop()
{
	while(!Serial.available());
	digitalWrite(LED, LOW);

	MSP_UART_to_LORA(rf95, Serial);

	if(rf95.waitAvailableTimeout(1000)){
		MSP_LORA_to_UART(rf95, Serial);
	}

	digitalWrite(LED, HIGH);

}
