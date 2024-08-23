
# Tandem-Badge
Tandem 50th Anniversary badge project

This project is a special anniversary badge that can be used for trade shows like the ones used at DefCon.  The standard for SAO badge boards is fully supported in the hardware and software
See: https://defcon.org/html/links/dc-badge.html
https://hackaday.io/project/175182-simple-add-ons-sao

Current software version is V3e (shown on the badge in the lower right corner sideways).   The current badge versio is 12x for both top and bottom. The two must match.  
The SAO boards have separate versioning and do not have to coinside (can be used on any other SAO enabled badge). 

The software has been enhanced to allow serial connection via USB port (115200) to prompt for values to place into persitent memory of the ESP-32. The use of the name loader tool is now unnecessary.

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
  - Sleep timeout in minutes.  How long until the device goes to sleep (range 1 to 999)

Badge software can be loaded and upgraded separately from the name load.  There is no need to use the name loader again once values are set properly. 

Badge software does the following:
Connects to WiFi. It tries the first SSID several times and the second one several times.  
Gets the time.   If fails, then the icon will not show connected in on the display. Time is updated every minute from internal clock.
Time is pulled from NTP pool every hour
Display is updated to reflect the name and employee number and status of the device. 
Switch management and sleep timer is set up (loops infinately).
See separate documentation for the switch management and sleep/time/display functions. 
Zip code and country code are not used. Eventually will be used for weather forecast display. 

General info:
Power is from the battery that sits under the top board. 
To recharge, you have to pull the display/cpu from the board (it is just plugged in).    Plug into the power source (PC or generic wall charger).  The estimate voltage is shown at the bottom of the screen but is unreliable because the drop form 4.1 to <3.1 (lowest that wil run the processor) is so quick that it is hard to catch from the power pins. BUt an estimate is attempted. and it looks cool. The number shown is only an estimate and the voltage sample is only taken via some trickery when the system starts up. At full charge it should be 4.1-4.07 volts.  It works fine down in the 3.1 volt range. 

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
Sleep timout value (1-9999 minutes) 

* Not yet used in the software. 

The configuration memory is loaded by setting all of the switches on "1111111111111111" and connecting to the serial port via USB.  The values stay on the device no matter which version of the badge software is loaded in the future. 

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

At boot the device attempts to connect to the first SSID, if it fails it tries the second.  If it connects, then it gets the time and offsets using the TZ parameter.  If the WIFi fails to connect, it shows N/A where time would normally reside. It will retry every hour.  
Time is displayed at the bottom. The wifi is then disconnected.  Time is maintained inside the device and is updated every minute. The Wifi is connected every hour to refresh and sync for clock float. 

Sleep logic: 
A timer is set that is a reflection of the timeout request in the Sleep timet parameter value. This paramter is in minutes. 
So, for example, if the timer is set for 15 minutes, then after 15 minutes with no switch being changed, the device will "sleep".
"Sleeping" is not sleeping the processor. The processor still needs to watch the switches. However, for practical purposes llittle power is used.  
While sleeping no LEDS show and no Wifi is used. Most of the power is being reserved.

Time logic:
Time is gathered via WiFI from an NTP server. Offset is presented and the HH:MM should reflect current time for the locale represented by the offset value. (-5 USEST, -6 USCST, -7 USMST, -8 USPST, etc.) 
Each minute the screen is refreshed with a new time. The refresh will be less obnoxious in a future release. Currently, the fulle screen is refreshed which causes it to blink.  
There is also a delay when startup happens (before the switches and LEDs are started). I do lots of nasty slow stuff at startup. 
After 30 minutes, the time is refreshed from the time server (via WiFI) So the net is that WiFi is only used every 30 minutes. 

#SAO ports and SAO boards

SAO ports are labeled "Dynabus: They are standard SAO V 1.68 ports. https://hackaday.com/2019/03/20/introducing-the-shitty-add-on-v1-69bis-standard/
with 3V, GND, SCL/SDA, GPIO1 and GPIO2 available.    The software uses 3V/GND, GPIO1 and GPIO2.  

The Chevron SAO uses 3V for fixed LEDs and GPIO2 for the candle blinking sequence for the other LEDs. 
The Mackie SAO uses 3V for the CPU LEDs and GPIO2 (and GOPI1) for the other LEDS using a candle display. 

The device will drive those pins for any SAO place onto the SAO header. Several have been tested to work. Even older designs will work as long as 3V and GND are next to each other. The circuit is protected against shorts.  

