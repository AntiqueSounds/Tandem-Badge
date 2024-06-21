
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

Badge software can be loaded and upgraded sepraretly from the name load.  There is no need to use the name loader again once values are set properly. 

Badge software does the following:
Connects to WiFi.  Gets the time.   If fails, then the icon will not show connected in on the display. Time is updated every minute from internal clock.
Time is pulled from NTP pool every hour
Display is updated to reflect the name and employee number and status of the device. 
Switch management and sleep timer is set up (loops infinately).
See separate documentation for the switch management and sleep/time/display functions. 
Longitude and Latitude are not yet used. Eventually will be used for weather forecast display. 
