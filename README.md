## This Arduino Project is to be used with iTrain Model Railroud control software. 
#### It can drive a model Platform-Clock and/or Platform-Train-Departure-Display.
Previously both display where available as dedicated projects, now both had been combined in one universal code.
The connection to the host is via USB COM port. A re implementatin of the telnet link which is known from previous project is in work.
At the moment the code is configured to drive both display type at the same time. It is already prepared to to support two or more displays of same type. This will added at a later state.
This is how the result look like. 
![image](https://user-images.githubusercontent.com/10500682/143782855-eb4b66c5-4b3a-4abf-8ddb-56ebdcacfe10.png)

The interface to iTrain is done with powershell scripts which are called from iTrain actions. 
They can be found in the ./script subfolder.
***
#### Platform Clock:
Parameters accepted by the setClockDisplay.ps1 Script:
'T' System   	... set the display to system time of the host computer, there MUST be a space after the 'T'
'T' 16:31    	... set the display to to given time, in thsi case 16:31, there MUST be a space after the 'T'
'S' 			... increment the time by by one minute

The sketch does only provide a static display, there is no realtime clock function included. 
This is to support the time scaling function from iTrain. 
To get the display synchronized with iTrain and the clock hands moving this action is recomended:
![image](https://user-images.githubusercontent.com/10500682/143782704-4e0a0537-3001-4f7e-9f89-4edbaf517cba.png)
***
#### Platform Display:
Parameter accepted by the setPlatformDisplay.ps1 Script:
The script accepts four subsequent parameters, parameters are separated by space 
Parameter1 First line. Train name
Parameter2 second line eg. first destination
Parameter3 third line eg. second destination
Parameter4 departure time
For german "umlauts" their representive must be used eg. "ae" instead of "ä"
eg setPlatformDisplay1.ps1 IEC123 Muenich Salzburg 13:30

Spaces within one parameter must be escaped by quotation marks.
eg setPlatformDisplay1.ps1 IEC123 Muenich 'ueber Salzburg' 13:30

![image](https://user-images.githubusercontent.com/10500682/143779301-96a357a8-8333-43ef-9490-4cd121c700d1.png)
***
The hardware is pretty simple. My prototype use a ARDUINO Micro, but for sure it can be any other Arduino type microcontroler.
The next version will use a ESP8266 NodeMCU board to support Wifi connection. The display are cheap OLED display with I2C interface. 
Wireing is straight forward, yust connect the SCL and SDA with any digital I/O Pin and configure the settings.h accordingly.

**The project is VSCode project with PlatformIO pluggin.**



