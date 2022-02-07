#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include "PubSubClient.h"

#include "settings.h"
#include "secrets.h"
#include "helpers.h" // all the calc fcuntions

#include <z21.h>

// declare the network instance
WiFiServer TelnetServer(port);
WiFiClient Telnet;
WiFiUDP Udp;
WiFiClient MQTTClient;
PubSubClient MQTTclient(MQTTClient);

// create z21 instance
z21Class z21;

// some globals
boolean CommandSourceSerial = false; // Receive commands from serial port (eg for debug)
boolean CommandSourceTelnet = true;
String commandStr;                 // telnet command
unsigned long llastMQTTUpdate = 0; // counter for hartbeat

// globals from the z21 lib
byte storedIP = 0; // speicher für IPs
typedef struct     // Rückmeldung des Status der Programmierung
{
  byte IP0;
  byte IP1;
  byte IP2;
  byte IP3;
} listofIP;
listofIP mem[maxIP];

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// use telnet to receive commands
void handleTelnet()
{
  if (TelnetServer.hasClient())
  {
    // client is connected
    if (!Telnet || !Telnet.connected())
    {
      if (Telnet)
        Telnet.stop();                   // client disconnected
      Telnet = TelnetServer.available(); // ready for new client
    }
    else
    {
      TelnetServer.available().stop(); // have client, block new conections
    }
  }

  if (Telnet.available())
  {
    // client input processing
    while (Telnet.available())
      commandStr = Telnet.readStringUntil('\n'); // read from Telnet
  }
}

//--------------------------------------------------------------------------------------------
// MQTT Callback
void callback(char *topic, byte *payload, unsigned int length)
{
  String sTopic = topic;
  String sPayload = ""; //(char *)payload;

  for (unsigned int i = 0; i < length; i++)
  {
    sPayload = sPayload + (char)payload[i];
  }

  Serial.print("MQTT Message: [");
  Serial.print(sTopic);
  Serial.print("] ");
  Serial.println(sPayload);

  Telnet.print("MQTT Message: [");
  Telnet.print(sTopic);
  Telnet.print("] ");
  Telnet.println(sPayload);

  // check Message type
  // Sensor status received, send Adress and payload to z21
  if (sTopic.indexOf("/Sensors/") > -1)
  {
    int iSensor = sTopic.substring(sTopic.indexOf("/Sensors/") + 9).toInt();
    int iAddr = trunc(iSensor / 8) + 1;
    int iPort = iSensor % 8 - 1;
    // Serial.println("SensorNr:" + String(iSensor) +  "  AdressNr: " + String(iAddr) + "  Port: " + String(iPort));
    if (sPayload.toInt() == 0)
    {
      z21.setCANDetector(1, iAddr, iPort, 0x11, 0x0000, 0);
      z21.setCANDetector(1, iAddr, iPort, 0x01, 0x0100, 0);
    }
    else if (sPayload.toInt() == 1)
    {
      z21.setCANDetector(1, iAddr, iPort, 0x11, 0x0000, 0);
      z21.setCANDetector(1, iAddr, iPort, 0x01, 0x1000, 0);
    }
    else
    {
      z21.setCANDetector(1, iAddr, iPort, 0x11, sPayload.toInt(), 0);
    }
  }

  // Accessory status received, send address and payload to z21
  if (sTopic.indexOf("/Accessories/") > -1)
  {
    int iAdr = sTopic.substring(sTopic.indexOf("/Accessories/") + 13).toInt();
    z21.setExtACCInfo(iAdr + 3, sPayload.toInt(), 0); 
  }
}

//--------------------------------------------------------------------------------------------
byte addIP(byte ip0, byte ip1, byte ip2, byte ip3)
{
  // suche ob IP schon vorhanden?
  for (byte i = 0; i < storedIP; i++)
  {
    if (mem[i].IP0 == ip0 && mem[i].IP1 == ip1 && mem[i].IP2 == ip2 && mem[i].IP3 == ip3)
      return i + 1;
  }
  if (storedIP >= maxIP)
    return 0;
  mem[storedIP].IP0 = ip0;
  mem[storedIP].IP1 = ip1;
  mem[storedIP].IP2 = ip2;
  mem[storedIP].IP3 = ip3;
  storedIP++;
  return storedIP;
}

//--------------------------------------------------------------------------------------------
// called from z21 lib when a ext accessory message is received from iTrain
void notifyz21ExtAccessory(uint16_t iAdr, byte state)
{

  Serial.println("iTrain Message  Accessory: " + String(iAdr - 3) + "  State: " + String(state));
  String sTopic;
  sTopic = hostname + "/Accessories/" + String(iAdr - 3);
  MQTTclient.publish(sTopic.c_str(), String(state).c_str());
}

//--------------------------------------------------------------------------------------------
// called from z21 lib when a Accessory message is received from iTrain
void notifyz21Accessory(uint16_t Adr, bool state, bool active)
{
  // todo: implement, but maybe can skip because working with ext accessory is way easier, but so far not clear how iTrain support
  // Serial.println("Accessory: " + String(Adr) + " State: " + String(state) + "Active: " + String(active)  );
}

//--------------------------------------------------------------------------------------------
// send z21 messages to iTrain
void notifyz21EthSend(uint8_t client, uint8_t *data)
{
  if (client == 0)
  { // all stored
    for (byte i = 0; i < storedIP; i++)
    {
      IPAddress ip(mem[i].IP0, mem[i].IP1, mem[i].IP2, mem[i].IP3);
      Udp.beginPacket(ip, Udp.remotePort()); // Broadcast
      Udp.write(data, data[0]);
      Udp.endPacket();
    }
  }
  else
  {
    IPAddress ip(mem[client - 1].IP0, mem[client - 1].IP1, mem[client - 1].IP2, mem[client - 1].IP3);
    Udp.beginPacket(ip, Udp.remotePort()); // no Broadcast
    Udp.write(data, data[0]);
    Udp.endPacket();
  }
}

//--------------------------------------------------------------------------------------------
// setup and module initialisation
void setup()
{
  // Init Serial
  Serial.begin(115200);

  // init EEprom
  EEPROM.begin(4096);

  delay(100);

  // connect to WiFi
  Serial.printf("Connecting to %s ", wifi_ssid);
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(hostname.c_str()); // define hostname
  WiFi.begin(wifi_ssid, wifi_password);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }

  // UDP Z21 Port
  Serial.println("Prepare UDP Port: ");
  Serial.println(z21Port);
  Udp.begin(z21Port);

  // OTA Stuff
  ArduinoOTA.setPort(iOTAPort);
  ArduinoOTA.setHostname(hostname.c_str());

  ArduinoOTA.onStart([]()
                     {
                       String type;
                       if (ArduinoOTA.getCommand() == U_FLASH)
                       {
                         type = "sketch";
                       }
                       else
                       { // U_FS
                         type = "filesystem";
                       }

                       // NOTE: if updating FS this would be the place to unmount FS using FS.end()
                       Serial.println("Start updating " + type); });

  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });

  ArduinoOTA.onError([](ota_error_t error)
                     {
                       Serial.printf("Error[%u]: ", error);
                       if (error == OTA_AUTH_ERROR)
                       {
                         Serial.println("Auth Failed");
                       }
                       else if (error == OTA_BEGIN_ERROR)
                       {
                         Serial.println("Begin Failed");
                       }
                       else if (error == OTA_CONNECT_ERROR)
                       {
                         Serial.println("Connect Failed");
                       }
                       else if (error == OTA_RECEIVE_ERROR)
                       {
                         Serial.println("Receive Failed");
                       }
                       else if (error == OTA_END_ERROR)
                       {
                         Serial.println("End Failed");
                       } });

  Serial.println("Starting OTA");
  ArduinoOTA.begin();

  // TELNET Stuff
  TelnetServer.begin();
  Serial.print("Starting telnet server on port " + (String)port);
  TelnetServer.setNoDelay(true); // ESP BUG ?
  Serial.println();
  delay(100);

  // MQTT Stuff
  MQTTclient.setServer(MQTTBroker, MQTTPort);
  MQTTclient.setCallback(callback);

  // ready to go
  Serial.println("Init done, starting main loop");
}

// the main loop
void loop()
{
  ArduinoOTA.handle();
  handleTelnet();

  // manage MQTT connection
  while (!MQTTclient.connected())
  {
    Telnet.println("Connect to MQTT Broker");
    Serial.println("Connect to MQTT Broker");
    MQTTclient.connect(hostname.c_str());
    MQTTclient.subscribe("iTrain2MQTT/Sensors/#");
    MQTTclient.subscribe("iTrain2MQTT/Accessories/#");
    delay(5000);
  }

  MQTTclient.loop();
  // push IP adress and uptime to MQTT Broker
  if (millis() - llastMQTTUpdate >= iHeartBeat)
  {
    String sTopic;
    sTopic = hostname + "/Device/IP Adresse";
    MQTTclient.publish(sTopic.c_str(), WiFi.localIP().toString().c_str());

    sTopic = hostname + "/Device/Uptime";
    char uptime[10];
    ltoa(millis(), uptime, 10);
    MQTTclient.publish(sTopic.c_str(), uptime);
    llastMQTTUpdate = millis();
  }

  //--------------------------------------------------------------------------------------------
  if (Udp.parsePacket() > 0)
  {                                              // packetSize
    Udp.read(packetBuffer, Z21_UDP_TX_MAX_SIZE); // read the packet into packetBufffer
    // Serial.print("Incomming connection from: ");
    // Serial.println(Udp.remoteIP());
    IPAddress remote = Udp.remoteIP();
    z21.receive(addIP(remote[0], remote[1], remote[2], remote[3]), packetBuffer);
  }

  // debug stuff, to be removed
  /*
   delay (200);
   z21.setTrntInfo(0,true);
   delay (200);
   z21.setTrntInfo(0,false);
   delay (200);
   z21.setTrntInfo(1,true);
   delay (200);
   z21.setTrntInfo(1,false);
*/

  // z21.setExtACCInfo(4,5,0xFF);
  /*
    z21.setExtACCInfo(4,0,0);
    delay (200);

    z21.setExtACCInfo(4,1,0);
    delay (200);

  z21.setExtACCInfo(4,2,0);
    delay (200);

    delay(100);
*/  

  yield();
}