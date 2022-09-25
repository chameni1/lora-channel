#include <SPI.h>
#include <LoRa.h>
#include "Arduino.h"
#include <PubSubClient.h>
#include "LoRaInterface.h"

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

// LoRa receive buffers
static bool _receivedFlag = false;
static String _payloadBuffer = "";

// WiFi credentials
#define WIFI_SSID     "*****"
#define WIFI_PASSWORD "*****"

// MQTT Broker info
// Define either an IP address...
//#define MQTT_SERVER   IPAddress(192, 168, 0, 2)  or a hostname
#define MQTT_SERVER   "Ameni"
#define USE_SPREAD_FACTOR 
static int _spreadFactor = MIN_SPREAD_FACTOR;


// LoRa Handling

void configureLoRa() {
  Serial.println("configureLoRa()");
#ifdef USE_SPREAD_FACTOR  
  LoRa.setSpreadingFactor(_spreadFactor);
#endif
  LoRa.onReceive(onReceive);
  LoRa.receive();
}

// LoRa receiver

// received signal strength

int rssi() {
  return LoRa.packetRssi();
}

// receiver ISR
static void onReceive(int packetSize)
{
  // it's an interrupt service routine
  digitalWrite(LED_BUILTIN, HIGH);
  // Copy LoRa payload into buffer 
  _payloadBuffer = "";
  while (LoRa.available())
  {
    _payloadBuffer += (char) LoRa.read();
  }
  // set flag to say there's data ready
  _receivedFlag = true;
  digitalWrite(LED_BUILTIN, LOW);
}

void setDefaultSpread() {
  LoRa.setSpreadingFactor(MIN_SPREAD_FACTOR);
}


// Check for recent incoming LoRa payload and copy it from the rxbuffer and pass it onto the caller.
String *checkRxBuffer() {
  static String payload;
  if (_receivedFlag && _payloadBuffer.length() > 0) {
    _receivedFlag = false;
    // ensure length does not exceed maximum
    int len = min((int)_payloadBuffer.length(), MAX_LORA_PAYLOAD-1);
    // copy String from rx buffer to local static variable for return to caller
    payload = _payloadBuffer;
    return &payload;
  } else {
    return NULL;
  }
}
// checkAndForwardPackets()
// This is the core function that checks for received LoRa packets and forwards the contents on to MQTT
//
static void checkAndForwardPackets() {
  // check for received data
  String *rxPacketString = checkRxBuffer();
  if (rxPacketString) {
    // forward packet content to MQTT
    const char *msg = rxPacketString->c_str();
  
    publishMQTT(msg);

    Serial.print("rx packet: msg: ");
    Serial.println(msg);

  }
}
void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
 while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
// Initialise wifi connection
  initWiFi(WIFI_SSID, WIFI_PASSWORD);
  
  // Configure LoRa interface
  configureLoRa();

  if (isWiFiConnected()) {
    connectToMQTTServer(MQTT_SERVER, 1883);
  }
  Serial.println("setup() done");
}
void loop() {

  // ensure WiFi stays connected
  checkWiFiStatus();

  // Perform packet forwarding
  checkAndForwardPackets();
  
  // MQTT housekeeping
  updateMQTT();

  delay(100);
}
