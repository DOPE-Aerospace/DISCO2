#ifndef MSP_FORWARDER
#define MSP_FORWARDER

void MSP_LORA_to_UART(RH_RF95& in, Stream& out);

void MSP_UART_to_LORA(RH_RF95& out, Stream& in);

#endif