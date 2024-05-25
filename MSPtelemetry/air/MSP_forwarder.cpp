#include <RH_RF95.h>

#include "MSP_forwarder.h"

static char buffer[RH_RF95_MAX_MESSAGE_LEN] = {};

void MSP_LORA_to_UART(RH_RF95& in, Stream& out)
{
	uint8_t received = {};
	do {
		in.recv(reinterpret_cast<uint8_t*>(buffer), &received);
		out.write(buffer, received);
	} while(received == RH_RF95_MAX_MESSAGE_LEN);
} 

void MSP_UART_to_LORA(RH_RF95& out, Stream& in)
{
	int first_byte = {};
	while((first_byte = in.read()) != '$'){
		if(first_byte == -1){
			return;
		}
	}
	buffer[0] = '$';

	if((buffer[1] = in.read()) == 'X'){

		in.readBytes(buffer + 2, 6);
		int response_lenght = 8 + buffer[6] + (buffer[7] * 256) + 1; //+1 per il byte di controllo

		if(response_lenght >= RH_RF95_MAX_MESSAGE_LEN){
			in.readBytes(buffer + 8, RH_RF95_MAX_MESSAGE_LEN - 8);
			out.send(reinterpret_cast<const uint8_t*>(buffer), RH_RF95_MAX_MESSAGE_LEN);
			response_lenght -= RH_RF95_MAX_MESSAGE_LEN;
			while(response_lenght >= RH_RF95_MAX_MESSAGE_LEN){
				in.readBytes(buffer, RH_RF95_MAX_MESSAGE_LEN);
				out.send(reinterpret_cast<const uint8_t*>(buffer), RH_RF95_MAX_MESSAGE_LEN);
				response_lenght -= RH_RF95_MAX_MESSAGE_LEN;
			}
			in.readBytes(buffer, response_lenght);
			out.send(reinterpret_cast<const uint8_t*>(buffer), response_lenght);
		} else {
			in.readBytes(buffer + 8, response_lenght - 8);
			out.send(reinterpret_cast<const uint8_t*>(buffer), response_lenght);
		}

	} else if(buffer[1] == 'M'){

		in.readBytes(buffer + 2, 3);
		int response_lenght = 5 + buffer[3] + 1; //+1 per il byte di controllo

		if(response_lenght >= RH_RF95_MAX_MESSAGE_LEN){
			in.readBytes(buffer + 5, RH_RF95_MAX_MESSAGE_LEN - 5);
			out.send(reinterpret_cast<const uint8_t*>(buffer), RH_RF95_MAX_MESSAGE_LEN);
			response_lenght -= RH_RF95_MAX_MESSAGE_LEN;
			while(response_lenght >= RH_RF95_MAX_MESSAGE_LEN){
				in.readBytes(buffer, RH_RF95_MAX_MESSAGE_LEN);
				out.send(reinterpret_cast<const uint8_t*>(buffer), RH_RF95_MAX_MESSAGE_LEN);
				response_lenght -= RH_RF95_MAX_MESSAGE_LEN;
			}
			in.readBytes(buffer, response_lenght);
			out.send(reinterpret_cast<const uint8_t*>(buffer), response_lenght);
		} else {
			in.readBytes(buffer + 5, response_lenght - 5);
			out.send(reinterpret_cast<const uint8_t*>(buffer), response_lenght);
		}

	}
}