## This project is a MQTT gateway for iTrain model railroad control software. 
#### Background is that most protocols used in model railroad environment are little bit complex and not easy to integrate in DiY uC projects. 
#### On the other hand the MQTT protocol is widely used in IoT applikations with tons of existing applikations and templates. So this can bring both together and allow easy implementation of special custom hardware with iTrain.

#### Implementation:
 For simplification the gateway is prepared to run on a ESP8266uC. This allow easy integration without struggling with PC operating system specialities. At later state a conversion to Java in order to run as service on any PC is thinkable but not planned yet.  
 The gateway connects to iTrain via Wifi by use of the Roco z21 protocol. From iTrain perspective it looks like a physical z21. Please consult the iTrain manual about z21 integration details. No special settings are necessary, all settings shall kept in default state. A second connection is initiated to a MQTT broker. 
 The MQTT broker need to be setup as standalone service, it is not part of the project. The IP address of the MQTT broker wheer the gateway shall connect is configurable. 

#### Project configuration:
The project is setup for VSCode IDE with the platformio framework installed. 
A initial firmware download to the uC need to be done via USB. Subsequent firmware downloads can be done via OTA (Wifi). Please consult the platformio.ini to select the prefered configuration.  Check this file also for mandatory third party libraries.  
Before initial compile you need to create a */src/security.h* file with your Wifi SSID and PIN. A sample for the syntax is provided as */src/security_example.h*
Further settings can be done in the /src/settings.h file. Minimum but mandatory setting is the broker IP address.

#### Limitations:
 iTrain does not offer a public interface to directly access internal objects. So a user based native integration is not possible. The workaround is to emulate a z21 command station which acts as a iTrain client and then push/receive messages to a MQTT broker. So this virtual z21 acts as a iTrain <> MQTT gateway.  
 This allow to mirror the status of any object assigend to the virtual z21 at a MQTT broker. Within iTrain this virtual objects can be used to trigger actions or to instance interact with physical objects assigned to other command stations.
Currently supported objects types:
- Set feedback status, including loco id. (simulate a railcom enabled feedback)
- Set accessory status, either boolean(0/1) or any integer number in range 0-256
- Get accessory status, either boolean(0/1) or any integer number in range 0-256

>Currently no security credentials are supported. Furthermore input validataion is very basic.
So it is stricly recomended to use the gateway only in protected environment. Do NOT link to a public 
MQTT broker. 

#### MQTT topic structur:
 Once connected to Wifi some default topics are created. The main Topic is always the hostname of the gateway (default: "iTrain2MQTT")  
 - *Hostname*/Device/IPAdress/*IP adress of the gateway*
 - *Hostname*/Device/Uptime/*GatewayUptime*
 - *Hostname*/Device/Uptime/*Version*
 
 Further topics are created once they are first time pushed. You may create it from your client. The gateway will automatically subscribe to all topics below this hirachy and forward it to the corresponding iTrain object:
 - *Hostaname*/Accessories/*AccessoryAdress*/*AccessoryState*
 - *Hostaname*/Sensors/*SensorAdress*/*SensorState*

pls. Note: Objcts are matches with their adress and not by name. eg:
iTrain2MQTT/Sensors/2/0    -> will set the iTrain feedback at virtual z21 with adress# 2 to "OFF"
iTrain2MQTT/Sensors/3/1    -> will set the iTrain feedback at virtual z21 with adress# 3 to "ON"
iTrain2MQTT/Sensors/3/123  -> will set the iTrain feedback at virtual z21 with adress# 3 to "ON" and report locoId 123 detected (simulate railcom behavior)

The same methode is used for accessories.
You may also create a accessory in iTrain and change the value. In thsi case the gateway creates the topic at the broker and push the value.

#### Debugging/Monitor:
As usual the debug output is send to serial monitor with 115200 baud.
A second debug output and and command input option is the use a telnet client.


#### Credits: 
The project use the Arduino z21 Library kindly provided by Philipp Gahtow. https://pgahtow.de/w/Z21_mobile/en

  
  
  
  
  

