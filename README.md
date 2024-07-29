
# Tandem-Badge
Tandem 50th Anniversary badge project 

The software has been enhanced to allow serial connection via USB port (115200) to prompt for values to place into persitent memory of the ESP-32. THe use of the name loader tool is now obsoleted. 

Serial port set to 115200. Prompts are: 
  - First name
  - Last name
  - SSID - Primary SSID for Wifi
  - WiFIPassword - Primary WiFI Password
  - SSID2 - Secondary WiFI SSID
  - WiFiPassword2 - Secondary WiFI Password
  - Employee number - Any text used for Employee number in the display.
  - Time zone offset from GMT (e.g. -5 for Eastern U.S. - default)
  - Country code (default = US - Not used in this release) 
  - Zip Code (not used in this software release)

Once memory is set up, then install the badge software.

Badge software can be loaded and upgraded separately from the name load.  There is no need to use the name loader again once values are set properly. 

Badge software does the following:
Connects to WiFi. It tries the first SSID 25 times and the second one 50 times.  
Gets the time.   If fails, then the icon will not show connected in on the display. Time is updated every minute from internal clock.
Time is pulled from NTP pool every hour
Display is updated to reflect the name and employee number and status of the device. 
Switch management and sleep timer is set up (loops infinately).
See separate documentation for the switch management and sleep/time/display functions. 
Zip code and country code are not used. Eventually will be used for weather forecast display. 

General info:
Power is from the battery that sits under the top board. 
To recharge, you have to pull the display/cpu from the board (it is just plugged in). This is a design flaw that will be resolved in the first real version.  Carefully pull it from the top but do not pull the battery plug out. Let it sit there.  Then plug into the power source (PC or generic wall charger)  The estimate voltage is shown at the bottom of the screen. The number shown is only an estimate and the voltage sample is only taken via some trickery when the system starts up. At full charge it should be 4-3.7 volts.  It works fine down in the 3 volt range. 

Software:

There is a persistent memory that holds:
First Name
Last Name
Employee number
Wifi SSID
WiFI Password
SSID2 - Secondary WiFI SSID
WiFiPassword2 - Secondary WiFI Password
Time zone offset from GMT (e.g. -5 for Eastern U.S. - default)
*Country code (default = US - Not used in this release) 
*Zip Code (not used in this software release)
*Weather key 

* Not yet used in the software. 

This memory is loaded by setting all of the switches on "1111111111111111" and connecting to the serial port via USB.  The values stay no matter which version of the badge software later load. 
Version 3d (Version 3e is in the works)

Power switch is at the bottom (reset button).   On to the left, off to the right. 
At boot (turn on) the names and information are loaded from memory (above). 
The display is refresh to a baseline name tag. 
After display is refreshed the LEDs are reset and the switches are processed. 

Switch usage: 
Normal switch on/off directs up to the led above it. On turns on the LED, off turns it off. 

Special switch values:

All switches on forces the device to prompt for memory values as listed above. Name, SSID, etc. 
Switches are considered big-endian from the user perspective (low order is rightmost (#16))

0000000000100010 = High-order switches +10 +14 – Octal 42, a random animated display.  There are 7 different possible random displays. 
0100000000000000 = Set to little endian "2" = octal “4000"    does a “cylon”  display 
1100000000000000 = Set to little endian “3” = octal "140000"  does a “boot startup display”

SAO devices are suppored on the “dynabus” ports
Software does a candle display on the SAO0 and SAO1 ports GPIO25 and GPIO26 respectively.    The direct power on 5V/G is supported too.   SISO is not used. 

The software pulsates the “throbbing” chevron in the lower right.   The power and run LEDs are also pulsed in the software (not related to the switches).

At boot the device attempts to connect to the first SSID, if it fails it tries the second.  If it connects, then it gets the time and offsets using the TZ parameter.
Time is displayed at the bottom. The wifi is disconnected.  Time is maintained inside the device and is updated every minute. THe Wifi is connected every hour to refresh and sync for clock float. 
