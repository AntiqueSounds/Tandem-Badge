
# Tandem-Badge
Tandem 50th Anniversary badge project 

Name Loader tool is used to place persitent values into the memory of the ESP-32. 
Install loader software and use the Serial port set to 115200. Prompts are: 
  - First name
  - Last name
  - SSID - Primary SSID for Wifi
  - WiFIPassword - Primary WiFI Password
  - SSID2 - Secondary WiFI SSID
  - WiFiPassword2 - Secondary WiFI Password
  - Employee number - Any text used for Employee number in the display.
  - Time zone offset from GMT (e.g. -5 for Eastern U.S.)
  - Longitude
  - Latitude
Once memory is set up, then install the badge software.

Badge software can be loaded and upgraded separately from the name load.  There is no need to use the name loader again once values are set properly. 

Badge software does the following:
Connects to WiFi. It tries the first SSID 25 times and the second one 50 times.  
Gets the time.   If fails, then the icon will not show connected in on the display. Time is updated every minute from internal clock.
Time is pulled from NTP pool every hour
Display is updated to reflect the name and employee number and status of the device. 
Switch management and sleep timer is set up (loops infinately).
See separate documentation for the switch management and sleep/time/display functions. 
Longitude and Latitude are not yet used. Eventually will be used for weather forecast display. 

General info:
Power is from the battery that sits under the top board. 
To recharge, you have to pull the display/cpu from the board (it is just plugged in). This is a design flaw that will be resolved in the first real version.  Carefully pull it from the top but do not pull the battery plug out. Let it sit there.  Then plug into the power source (PC or generic wall charger)  The estimate voltage is shown at the bottom of the screen. The number shown is only an estimate and the voltage sample is only taken via some trickery when the system starts up. At full charge it should be 3-3.3 volts.  It works fine down in the 1.8 volt range. 

Software:

There is a persistent memory that holds:
First Name
Last Name
Employee number
Wifi SSID
WiFI Password

This memory is loaded in a separate tool.  It stays no matter which version of the badge software later load. 
Version 3b (Version 3c is in the works)

Power switch is at the bottom (reset button).   On to the left, off to the right. 
At boot (turn on) the names and information are loaded from memory (above). 
The display is refresh to a baseline Name tag. 
After display is refreshed the LEDs are reset and the switches are processed. 

Switch usage: 
Normal switch on/off directs up to the led above it. On turns on the LED, off turns it off. 

Special switch values:

Low-order switches +0 + 1 + 2 all three on puts the device to sleep.   This is a testing switch for me to develop sleeping logic and display.   Device will sleep 5 minutes. When it wakes, it is like a reboot. 

High-order switches +13 +14 +15 – All three on, a random animated display.  There are 7 different possible random displays. 
Set to “5”   010    does a “cylon”  display
Set to “3”  011     does a “boot startup display”

SAO devices are suppored on the “dynabus” ports
Software does a candle display on the SAO port GPIO1.    The direct power on 5/G is supported too.   GPIO0 and SISO is not used. 

The software pulsates the “throbbing” chevron in the lower right.   The power and run LEDs are also pulsed in the software (not related to the switches).

