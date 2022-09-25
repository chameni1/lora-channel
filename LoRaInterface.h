
#ifndef __LORA_INTERFACE
#define __LORA_INTERFACE

#define MAX_LORA_PAYLOAD  256

#define MIN_SPREAD_FACTOR (7)   
#define MAX_SPREAD_FACTOR (12)

#define MAX_TX_POWER      (20)

void configureLoRa();
String *checkRxBuffer();
int rssi();

#endif //  __LORA_INTERFACE
