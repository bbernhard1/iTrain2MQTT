
// Wifi Settings
String hostname = "iTrain2MQTT";   //if change then must also update platformio.ini

// Telnet Settings
int port = 23;

// OTA Settings
int iOTAPort = 8266; // ESP32: 3232, ESP8266: 8266

// MQTT Settings
const char* MQTTBroker = "10.0.0.1";  // IPS Server
const int MQTTPort = 1883;            // IPS MQTT Port
const int iHeartBeat = 5000;          // Intervall for sending the uptime to MQTT broker 

// Settings for z21 Lib
#define Z21_UDP_TX_MAX_SIZE 15  //--> POM DATA has 12 Byte!
unsigned char packetBuffer[Z21_UDP_TX_MAX_SIZE +1]; //buffer to hold incoming packet,
#define maxIP 20  //Größe des IP-Speicher

