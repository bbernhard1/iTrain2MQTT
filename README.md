## This project is a MQTT gateway for iTrain model railroad control software. 
#### Background is that most protocols used in model railroad enviropnment are little bit complex and not easy to integrate in Diy uC projects. 
#### On the other hand the MQTT protocol is widely used in IoT applikations with tons of existing applikations and templates. So this can bring both together and allow easy creation of new iTrain functions.
Implementation: For simplification the gateway is prepared to run on a ESP8266uC. This allow easy integration without struggling with PC operating system specialities. At later state a conversion to Java in order to run as service on any PC is thinkable but not planned yet. It connects to iTrain via Wifi, same way as a physical z21 might do. Please consult the iTrain manual about details. No special settiungs necessary, all settings can kept in their default state.
There is no MQTT broker included. The broker need to be setup as standalone serice it is not part of the project. The IP adress of the MQTT broker is configurable. No security credentials are not supported at the moment. So its stricly recoemnded to use onyl in protected environment.

Project configuration: The project is setup for VSCode IDE with the platformio framework installed. 
A first initial download to the uC need to be done via USB. Subsequent downloads can be done via OTA (Wifi). Please consult the platformio.ini to select the prefered configuration.  
Before initial compile you need to create a /src/security.h file with your Wifi SSID and PIN. A sample fro the syntax is provided as /src/security_example.h.
Furthermore settings can be done in the /src/settings.h file. Minimum but mandatory setting is the IP adress of your MQTT broker.

Limitations: iTrain does not offer a public interface to directly access internal objects. So a user basedd native integration is not possible. The workaround is to emulate a z21 command station which acts as a iTrain client and push/receive messages to a MQTT broker. So this virtual z21 acts as a iTrain <> MQTT gateway.
This allow to mirror the status of any object assigend to this virtual z21 at a MQTT broker. Within iTrain this virtual objects can be used to trigger actions or for for instance interact interact with physical objects assigened to other command stations.
Currently supported objects:
- Set feedback status, including loco id. (simulate a railcom enabled feedback)
- Set accessory status, either boolean(0/1) or any integer number in range 0-256
- Get accessory status, either boolean(0/1) or any integer number in range 0-256

MQTT structured: Once connected to Wifi it creates the following default topics. main Topic is always the hostname of teh gateway (default: todo)
<Hostname>/Device/IPADRESS/<IP adress of teh gateway>
                 /Uptime/<uptime since gateway boot>
Further topics are created once first time pushed. For thsi you yust need to create it in iTrain, and assign it to the virtual z21. Once you change the status of teh accessory a topic following this scheme should appear on the broker:
<Hostaname>/Accessorys/<Accessory#>/<Accessory State>
They are automatically subscribed by the gateway and changes are forwarded to the linked iTrain object.  
To link a sensor you need to create a topic following this scheme. Once created it is subscribed automatically and linked to the iTrain sensor with same # 
<Hostaname>/Sensors/<Sensor#>/<Sensor State>

  
  
  
  
  

