
# Tandem-Badge
## Tandem 50th Anniversary badge project ✨

This project is a special anniversary badge that can be used for trade shows like the ones used at DefCon.  The standard for SAO badge boards is fully supported in the hardware and software
See:   
[DefCon badges](https://defcon.org/html/links/dc-badge.html)  
[SAO board specs](https://hackaday.io/project/175182-simple-add-ons-sao)

*Note: I am working on instructions for building the device.This will be done in a while. I have to take photographs of each step and then document each step. This will take me a bit of time. Please be patient.*

Current software version is V3E (shown on the badge in the lower right corner sideways).   The current badge versio is 12x for both top and bottom. The two boards must match versions.  
The SAO boards have separate versioning and do not have to coincide (can be used on any other SAO enabled badge). 


**The switches are FRAGILE, but functional.  Please gently switch them. This was the most difficult part of the whole project. The design and assembly of these switch extensions is still evolving.** 

## Functionality
The software has been enhanced to allow serial connection via USB port (115200) to prompt for values to place into persistent memory of the ESP-32. The use of the name loader tool is not unnecessary anymore.

Serial port is set to 115200. The USB is for powerand for serial communication. (Configuration or debugging.) Serial Prompts are for the values shown below in the software persistent memory section. 

Badge software updates can be loaded and upgraded separately from the name load.  There is no need to use the name loader tool. Once set, they persist until cleared manually or over-written by this software. 

Badge software does the following:
Connects to WiFi. It tries the first SSID several times and the second one several times.  
Gets the time.   If fails, then the icon will not show connected in on the display. Time is updated every minute from internal clock.
Time is pulled from NTP pool every 30 minutes
Display is updated to reflect the name and employee number and status of the device. 
Switch management and sleep timer is set up (loops infinately).
See separate documentation for the switch management and sleep/time/display functions. 
Zip code and country code are not yet used. Eventually will be used for weather forecast display. 

### General info:
Power is from the battery that sits under the top board. 
To recharge, you plug the USB into the power source (PC or generic wall charger).  The estimated voltage is shown at the bottom of the screen but is unreliable because the drop form 4.1 to <3.1 (lowest that will run the processor) is so quick that it is hard to catch from the power pins. But an estimate is attempted and it looks cool. The number shown is only an estimate and the voltage sample is only taken via some trickery when the system starts up. At full charge it should be 4.1-4.07 volts.  It works fine down in the 3.1 volt range. 

### Software:

There is a persistent memory that holds:
- First Name
- Last Name
- Employee number
- Wifi SSID
- WiFI Password
- SSID2 - Secondary WiFI SSID
- WiFiPassword2 - Secondary WiFI Password
- Time zone offset from GMT (e.g. -5 for Eastern U.S. - default)
- *Country code (default = US - Not used in this release) 
- *Zip Code (not used in this software release)
- *Weather key 
- Sleep timout value (1-9999 minutes)
    
*Not yet used in the software. 

The configuration memory is loaded by setting all of the switches on "1111111111111111" and connecting to the serial port via USB.  The values stay on the device no matter which version of the badge software is loaded in the future. 

Power switch is at the bottom (reset button).   On to the left, off to the right. 
At boot (turn on) the names and information are loaded from memory (above). 
The display is refreshed to a baseline name tag. 
After display is refreshed the LEDs are reset and the switches are processed. 

### Switch usage: 
Normal switch on/off directs up to the led above it. On turns on the LED, off turns it off. 

#### Special switch values:

All switches on forces the device to prompt for memory values as listed above. Name, SSID, etc. 
Switches are considered big-endian from the user perspective (low order is rightmost (#16))

0000000000100010 = High-order switches +10 +14 – Octal 42, a random animated display. There are 7 different possible random displays.  
0100000000000000 = Set to little endian "2" = octal “4000"    does a “cylon”  display  
1100000000000000 = Set to little endian “3” = octal "140000"  does a “boot startup display”

The software pulsates the “throbbing” chevron in the lower right.   The power and run LEDs are also pulsed in the software (not related to the switches).

At boot the device attempts to connect to the first SSID, if it fails it tries the second.  If it connects, then it gets the time and offsets using the TZ parameter.  If the WIFi fails to connect, it shows N/A where time would normally reside. It will retry every hour.  
Time is displayed at the bottom. The wifi is then disconnected.  Time is maintained inside the device and is updated every minute. The Wifi is connected every hour to refresh and sync for clock float. 
### Display layout:
The display places the First name as the largest text. Text size adjusts as the length requires. Up to 5 characters show at the largest size.  Last name is placed below and is resized according to how long it is. The maximum viable last name length is about 14 characters (if I rememeber correctly?).  
Lowest line shows an estimate of battery power. Do not rely entirely on this image or voltage shown. If it drops below 3.9, you should recharge the device.  
The employee number value is display to the right of the battery.   
The time is shown if available. If not, it should show "N/A".  The device requires access to WiFi in order to obtain the time.  
To the right of the time is icon(s) that show state.  If in flux getting the time, it will change a few times.  Once the device is settled and resting, the icon should be a little handheld device image.  When sleeping, it shoulds show a little crecent moon. 
To the right of the icons is tiny static informational text:  Version id, a number showing the current sleep time setting, and the name of the prefered SSID to connect to.  
The screen refreshes every minute (time display updates each minute). 

### Sleep logic: 
A timer is set that is a reflection of the timeout request in the Sleep time parameter value. This parameter is represented in minutes. 
For example, if the timer is set for 15 minutes, then after 15 minutes with no switch being changed, the device will "sleep".
"Sleeping" is not sleeping the processor. The processor still needs to watch the switches. However, for practical purposes little power is used.  
While sleeping no LEDS show and no Wifi is used. Most of the power is being reserved.

### Time logic:
Time is gathered via WiFI from an NTP server. Offset is presented and the HH:MM should reflect current time for the locale represented by the offset value. (-5 USEST, -6 USCST, -7 USMST, -8 USPST, etc.) 
Each minute the screen is refreshed with a new time. The refresh will be less obnoxious in a future release. Currently, the full screen is refreshed which causes it to blink.  
There is also a delay when startup happens (before the switches and LEDs are started). I do lots of nasty slow stuff at startup. 
After 30 minutes, the time is refreshed from the time server (via WiFI) So the net is that WiFi is only used every 30 minutes. 

## SAO ports and SAO boards
### SAO devices are supported on the “dynabus” ports
Software does a candle display on the SAO0 and SAO1 ports GPIO25 and GPIO26 respectively.    The direct power on 5V/G is supported too.   SISO is not used. 

SAO ports are labeled "Dynabus: They are standard SAO V 1.68 ports. https://hackaday.com/2019/03/20/introducing-the-shitty-add-on-v1-69bis-standard/
with 3V, GND, SCL/SDA, GPIO1 and GPIO2 available.    The software uses 3V/GND, GPIO1 and GPIO2.  

The Chevron SAO uses 3V for fixed LEDs and GPIO2 for the candle blinking sequence for the other LEDs. 
The Mackie SAO uses 3V for the CPU LEDs and GPIO2 (and GOPI1) for the other LEDS using a candle display. 

The device will drive those pins for any SAO place onto the SAO header. Several have been tested to work. Even older designs will work as long as 3V and GND are next to each other. The circuit is protected against shorts.  

