/*
Tandem name badge simulation software 
M. Keith Moore
*/
/*****************************************************************************************
* This is the linty version 5 (3f) of the Tandem Badge software. 
*  Fixed a bug where the second largest first name font was bad.  Fixed to latin28pt line 1116
* Next version will be delinted and will add a few more features.

* Stored values are: 
* First Name
* Last Name
* Employee number (any string)
* SSID primary name
* WiFi Password primary
* SSID backup name
* WiFI backup name
* Timezone offset value (hours offset from GMT)  -5 is US eastern
* Zip code for mapping weather location   Not used
* Country code (defaults to US) for future use for weather   Not used 
* Open Weather Key - for future weather update usage  Not used
* Sleep timer - How many minutes with no activity before sleeping. (1-999) Any key toggle re-activates.  
*
*
*
* Functionally, the setup clears the screen and estsblished the pinouts for the shifters and switches.  It draws the baseline display without the time  
* 
* The loop is: 
*      - set display
*      - Set GPIO led blinkies (reset, power, and throbber)
*      - Check switches
*      - Map switches to actions
*      - Switches are reflected above their locaiotn in the LED or..
*      - Switches do special actions.  Documented elsewhere.
*      - Check timers - Sleep (below) and minute update times. Every 30 minutes (default) the NTP time will be resynced with the internal clock.  
*      - Sleep time is checked and reset each time a switch is changed. Wake is done by flipping any switch. 
*        Sleep only stops the LEDs.  THe WiFi is not active except every time the time is synced to NTP every 30 minutes
*  
*  
*************************************************************************/


// Libraries are sensitive to order.   Take care moving things around. 

#include <JSON_Decoder.h>
#include <OpenWeather.h>

//#include <JSON_Listener.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <Timers.h>
#include <jled.h>
//#include <ESPHTTPClient.h>
#include <time.h>
#include <Adafruit_GFX.h>
//#define ESP32

#define LILYGO_T5_V213


//#include "SunMoonCalc.h"
/*
Tandem name badge simulation software 
M. Keith Moore
*/
//#define DEBUG TRUE // Comment this out for non-debug
#define ENABLE_GxEPD2_GFX 0
#define DISVERSION "V3F"
// Version "F" fixed a first name display issue with the font for fnames over 6 characters. 
#define XSTATUS 237  // beginning of the status @ this column
#define YSTATUS 126  // beginning of the status at this row
#define BOXWIDTH 200
#define BOXHEIGHT 22
#define DATA_WIDTH 16
#define uS_TO_S_FACTOR 1000000 // Conversion factor for micro seconds to seconds 

//#ifdef DEBUG
#define TIME_TO_SLEEP 5 // How long ESP32  sleep (in seconds) 5 seconds for testing. - No longer used
//#else
//#define TIME_TO_SLEEP 1800 // How ESP32 will sleep for (in seconds) 30 minutes
//#endif

#define SERIALTIMEOUT 1000
#define BASELINELEDREFRESH 100
#define REFRESHTIMERVAL 60000  // 1 minute (timer in ms) 

// Values here are little endian. Values on the panel are big endian. 
//#define UNIVERSALNUMBER 0B0000000000100010 // normally this should be octal 42. switches set to this turns on the blinkies
#define UNIVERSALNUMBER 0B0100010000000000 // normally this should be octal 42 (big endian on the panel). switches set to this turns on the blinkies

//#define UNIVERSALNUMBER   0B1110000000000000 // this is a test value 7 (the are little endian left side- forced here to right side) 
#define         CYLON   0B0000000000000101 // this is a test value 5 (the are little endian left side- forced here to right side in the code) 
#define LOADREGISTERS   0B0000000000000011 // this is a test value 3 (the are little endian left side- forced here to right side in the code) 
#define SLEEPSWITCHEDON 0B1000000000000001 // high+low value makes me sleepy.
#define     GETNEWPREFS 0B1111111111111111 // high values sets load preferences on serial port  
#define NOWIFINOW "O"  // was o or a
#define YESWIFINOW "l" // these are special characters for the special fonts (meteo and others) 
#define FAILWIFI "n"
#define SLEEPWIFI "2"

#include <Preferences.h>
#include <boards.h>

//#include <GxEPD2_3C.h>
//#include <GxEPD2_4C.h>
//#include <GxEPD2_7C.h>

//#include <GxEPD2_3C.h>
//#include <GxEPD2_4C.h>
//#include <GxEPD2_7C.h>
//#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w  form DKE GROUP
#include <GxEPD.h>
#include <GxEPD2_BW.h>
#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w  from DKE GROUP
#define GxEPD2_DRIVER_CLASS GxEPD2_213_BN // DEPG0213BN  122x250, SSD1680, (FPC-7528B), TTGO T5 V2.4.1, V2.3.1
//#define GxEPD2_DRIVER_CLASS GxEPD2_213_GDEY0213B74 // GDEY0213B74 122x250, SSD1680, (FPC-A002 20.04.08)

//#include <SD.h>
//#include <FS.h>
//#include <WiFi.h>

//#include GxEPD_BitmapExamples
// FreeFonts from Adafruit_GFX
// Fonts are located in the libraries/fonts directory under the Adafruit_GFX_Library folder in the fonts folder or in GxEPD2. 

#include <Fonts.h>  // the files called Fonts contains the used list instead of keeping it here. 

/* These should reside in the GFX library shown here still to confirm the ones that are used and the ones not used. 
* See the fonts.h file to see which ones are active and used

#include <Fonts/data_latin6pt7b.h>
#include <Fonts/data_latin8pt7b.h>
#include <Fonts/data_latin10pt7b.h>
#include <Fonts/data_latin12pt7b.h>
#include <Fonts/data_latin18pt7b.h>
#include <Fonts/data_latin20pt7b.h>
#include <Fonts/data_latin22pt7b.h>
#include <Fonts/data_latin24pt7b.h>
#include <Fonts/data_latin28pt7b.h>
#include <Fonts/data_latin36pt7b.h>
#include <Fonts/data_latin42pt7b.h>
//#include <Fonts/FreeMonoBold36pt7b.h>
//#include <Fonts/FreeMonoBold42pt7b.h>
//#include <Fonts/FreeMonoBold9pt7b.h>
//#include <Fonts/FreeMonoBold12pt7b.h>
//#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMono24pt7b.h>
#include <Fonts/FreeMono18pt7b.h>
//#include <Fonts/FreeSansOblique24pt7b.h>
//#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/TomThumb.h>
#include <Fonts/meteocons7pt7b.h>
#include <Fonts/meteocons10pt7b.h>
#include <Fonts/meteocons12pt7b.h>
#include <Fonts/meteocons14pt7b.h>
#include <Fonts/meteocons16pt7b.h>
#include <Fonts/WIFI4pt7b.h>
#include <Fonts/WIFI6pt7b.h>
#include <Fonts/WIFI8pt7b.h>
#include <Fonts/WIFI10pt7b.h>
#include <Fonts/WIFI12pt7b.h>
// 'wifi_PNG62355', 20x14px
// 'wifi_PNG62355', 20x14px
*/


const unsigned char epd_bitmap_wifi_PNG62355 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 64)
const int epd_bitmap_allArray_LEN = 1;
const unsigned char* epd_bitmap_allArray[1] = {
	epd_bitmap_wifi_PNG62355
};
//#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
// PL pin 1 
int load =14;   
// CE pin 15       
int clockEnablePin = 4;  
// Q7 pin 7  
int dataIn = 34;   
 // CP pin 2  
int clockIn = 2;   
//flags and toggles
bool alternate = false; // set up for use of two different displays 
bool specialSwitchSet = false; // if special register switches are set for special display features. 
bool noReverse = false;
bool timeToSleep = false; // toggle to put the unit to sleep for TIME_TO_SLEEP amount of time 
bool onlineReady = false;  // used to tell if the WiFi is connected. 
bool timeIsReal = false; // This is returned from the getTime routine and holds that state of the time field validity.
// Define Connections to 74HC595
// ST_CP pin 12
const int latchPin = 27;
// SH_CP pin 11
const int clockPin = 12;
// DS pin 14
const int dataPin = 0; 
 // Define data array
int16_t ledArray[16];
int ledCounter = 0;  // used to hold the current array element 
int ledRefreshDelay = 100; // milliseconds of led time shown
uint16_t ledResult = 0;
uint16_t result, lastResult = 0;  // current switch values
bool flipflop = true;  // used to reversing the LED displays 
int sleepTimeout = 15;
#ifdef DEBUG
int sleepTimerVal = 120000;  // in ms - 2 minute for testing 
#else
int sleepTimerVal= 3600000;  //  1 hour
#endif

//const int vPin = 35;  // was 13 but the board shows pin 35 as battery test? 
const int vPin = 39;  // This is an open pin and can hold voltage-  board shows pin 39 as battery test but no.  
const int throbberPin = 13;  // chevron pin 
const int SAO1Pin = 25;  // SAO pin
const int SAO2Pin = 26;  //  SAO pin
const int resetPin = 32; // Reset LED pin
const int powerPin = 33; // power LED pin
const int LEDGPin = 22; // power LED pin

//GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
//GxEPD_Class display(io, EPD_RSET, EPD_BUSY);
GxEPD2_BW<GxEPD2_290_GDEY029T94, GxEPD2_290_GDEY029T94::HEIGHT>
display(GxEPD2_290_GDEY029T94(/*CS=5*/ EPD_CS, /*DC=*/ EPD_DC, /*RST=*/ EPD_RSET, /*BUSY=*/ EPD_BUSY)); // GDEY029T94  128x296, SSD1680, (FPC-A005 20.06.15)

// Setup software PWM for leds. 


auto throbberled = JLed(throbberPin).Breathe(1800).DelayAfter(150).MaxBrightness(75).Forever(); // changed from 20 to 50 vf
//auto throbberled = JLed(throbberPin).Candle(5 /* speed */, 150 /*jitter*/).Forever();
//auto resetled = JLed(resetPin).Breathe(3000).MinBrightness(10).DelayAfter(500).Repeat(50);
//auto powerled = JLed(powerPin).Breathe(2500).MinBrightness(10).DelayAfter(500).Repeat(50);
//auto resetled = JLed(resetPin).Breathe(2000).MaxBrightness(30).MinBrightness(5).DelayAfter(500).Forever();
//auto powerled = JLed(powerPin).Breathe(3000).MaxBrightness(30).MinBrightness(5).DelayAfter(500).Forever();
auto resetled = JLed(resetPin).Breathe(5000).MaxBrightness(10).Forever();
auto powerled = JLed(powerPin).Breathe(7000).MaxBrightness(10).Forever();
auto SAO1led = JLed(SAO1Pin).Candle(4 /* speed */, 100 /*jitter*/).Forever();
auto SAO2led = JLed(SAO2Pin).Candle(6 /* speed */, 200 /*jitter*/).Forever();
/*
auto throbberled = JLed(jled::Esp32Hal(throbberPin,4)).Breathe(1800).DelayAfter(150).MaxBrightness(20).Forever();
auto resetled = JLed(jled::Esp32Hal(resetPin,3)).Breathe(5000).MaxBrightness(10).Forever();
auto powerled = JLed(jled::Esp32Hal(powerPin,2)).Breathe(7000).MaxBrightness(10).Forever();
auto SAO2led = JLed(jled::Esp32Hal(SAO2Pin,1)).Blink(1000,1000).Forever();
auto SAO1led = JLed(jled::Esp32Hal(SAO1Pin,0)).Blink(1000,1000).Forever();
*/
//auto boardled = JLed(LEDGPin).Breathe(1800).DelayAfter(150).MaxBrightness(20).Forever();
// Setup timer(s)

Timers refreshTimer;
Timers sleepTimer;
// Wifi Time (NTP setup)

// set up weather
OW_Weather ow; // Weather forecast library instance

//const char* ntpServer = "pool.ntp.org";
unsigned long lastUpdate = 0;
unsigned long lastIntervalUpdate = 0;
const long updateInterval = 18000000; // how often to update time seconds 30 mins
unsigned long lastNTPTime = 0;
const char* ntpServer = "time.nist.gov";
int  gmtOffset_hours = -4;
const long  gmtOffset_sec = gmtOffset_hours * 3600;
const int   daylightOffset_sec = 60000;
// location for weather
float longitude = 44.537;
float latitude = -85.3607;


WiFiUDP udpClient;
NTPClient timeClient(udpClient,ntpServer,gmtOffset_sec,daylightOffset_sec);

// See https://docs.thingpulse.com/how-tos/openweathermap-key/
String OPEN_WEATHER_MAP_APP_ID = "ec5f1053c471c0d461c4b2ab3e75745e"; // this is my personal used as a defauly
String OPEN_WEATHER_MAP_LANGUAGE = "en";
//boolean IS_METRIC = true;// PLEASE GET YOUR OWN AND ENTER IT INTO THE PREFS memory using the configuration process. 
/*
Go to https://openweathermap.org/find?q= and search for a location. Go through the
result set and select the entry closest to the actual location you want to display 
data for. It'll be a URL like https://openweathermap.org/city/2657896. The number
at the end is what you assign to the constant below.
 */
String OPEN_WEATHER_MAP_LOCATION_ID = "49633";
/*
Arabic - ar, Bulgarian - bg, Catalan - ca, Czech - cz, German - de, Greek - el,
English - en, Persian (Farsi) - fa, Finnish - fi, French - fr, Galician - gl,
Croatian - hr, Hungarian - hu, Italian - it, Japanese - ja, Korean - kr,
Latvian - la, Lithuanian - lt, Macedonian - mk, Dutch - nl, Polish - pl,
Portuguese - pt, Romanian - ro, Russian - ru, Swedish - se, Slovak - sk,
Slovenian - sl, Spanish - es, Turkish - tr, Ukrainian - ua, Vietnamese - vi,
Chinese Simplified - zh_cn, Chinese Traditional - zh_tw.
*/






//  THese are obtaned from the persistant memory prefs
String firstName;
String lastName;
String password;
String ssid;
String password2;
String ssid2;
String empNo;
String zipCode = "49633";
String countryCode = "US";
String units = "imperial";

String openWeatherKey = "ec5f1053c471c0d461c4b2ab3e75745e";
int TZ= -5;  // used ony for loadig TZ value. 

// Time and refresh time timers
    unsigned long currentMill = millis(); // grab the time 
    const long displayInterval = 60000; // minute
    unsigned long currentMillSinceLastUpdate = ((currentMill-lastUpdate)%1000);
    unsigned long currentSeconds = lastNTPTime + ((currentMill - lastUpdate)/1000);

// Set up to use flash memory preference library

// Screen display buffers
String input;
String partialScreenBuffer = "      ";
String wifiStatus = FAILWIFI;  // this will be the WIFI font character representing the current Wifi state (a, b, c, etc.) 
// worker values for voltage 
float volts = 0;
float percentage = 0;

Preferences prefs;

//Tandem Computers logo 

// 'Tandem_Chevron-small', 35x34px
//const unsigned char epd_bitmap_Tandem_Chevron_small [] PROGMEM = {
const uint8_t epd_bitmap_Tandem_Chevron [] PROGMEM = {
	0xff, 0xff, 0x07, 0x83, 0xe0, 0xff, 0xfc, 0x07, 0x03, 0xe0, 0xff, 0xf8, 0x1c, 0x07, 0xe0, 0xff, 
	0xf0, 0x38, 0x0e, 0xe0, 0xff, 0xe0, 0x70, 0x1c, 0xe0, 0xff, 0x80, 0xe0, 0x38, 0xe0, 0xff, 0x01, 
	0xc0, 0x70, 0xe0, 0xfe, 0x03, 0x80, 0xf0, 0xe0, 0xfc, 0x07, 0x01, 0xf0, 0xe0, 0xf8, 0x0e, 0x03, 
	0xb0, 0xe0, 0xf0, 0x1c, 0x07, 0x30, 0xe0, 0xe0, 0x38, 0x0e, 0x30, 0xe0, 0x80, 0xf0, 0x1c, 0x30, 
	0xe0, 0xff, 0xff, 0xf8, 0x30, 0xe0, 0xff, 0xff, 0xf0, 0x30, 0xe0, 0xff, 0xff, 0xf0, 0x30, 0xe0, 
	0xff, 0xff, 0xf0, 0x30, 0xe0, 0xff, 0xff, 0xf0, 0x30, 0xe0, 0xff, 0xff, 0xf0, 0x30, 0xe0, 0xff, 
	0xff, 0xf0, 0x31, 0xe0, 0xff, 0xff, 0xf0, 0x33, 0xe0, 0xff, 0xff, 0xf0, 0x33, 0xe0, 0xff, 0xff, 
	0xf0, 0x37, 0xe0, 0xff, 0xff, 0xf0, 0x3f, 0xe0, 0xff, 0xff, 0xf0, 0x3f, 0xe0, 0xff, 0xff, 0xf0, 
	0x3f, 0xe0, 0xff, 0xff, 0xf0, 0x3f, 0xe0, 0xff, 0xff, 0xf0, 0x7f, 0xe0, 0xff, 0xff, 0xf0, 0xff, 
	0xe0, 0xff, 0xff, 0xf1, 0xff, 0xe0, 0xff, 0xff, 0xf3, 0xff, 0xe0, 0xff, 0xff, 0xf3, 0xff, 0xe0, 
	0xff, 0xff, 0xf7, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xe0
};
// 'Tandem_Chevron-inverted', 35x34px
const uint8_t epd_bitmap_Tandem_Chevron_inverted [] PROGMEM = {
	0x00, 0x00, 0xf8, 0x7c, 0x00, 0x00, 0x03, 0xf8, 0xfc, 0x00, 0x00, 0x07, 0xe3, 0xf8, 0x00, 0x00, 
	0x0f, 0xc7, 0xf1, 0x00, 0x00, 0x1f, 0x8f, 0xe3, 0x00, 0x00, 0x7f, 0x1f, 0xc7, 0x00, 0x00, 0xfe, 
	0x3f, 0x8f, 0x00, 0x01, 0xfc, 0x7f, 0x0f, 0x00, 0x03, 0xf8, 0xfe, 0x0f, 0x00, 0x07, 0xf1, 0xfc, 
	0x4f, 0x00, 0x0f, 0xe3, 0xf8, 0xcf, 0x00, 0x1f, 0xc7, 0xf1, 0xcf, 0x00, 0x7f, 0x0f, 0xe3, 0xcf, 
	0x00, 0x00, 0x00, 0x07, 0xcf, 0x00, 0x00, 0x00, 0x0f, 0xcf, 0x00, 0x00, 0x00, 0x0f, 0xcf, 0x00, 
	0x00, 0x00, 0x0f, 0xcf, 0x00, 0x00, 0x00, 0x0f, 0xcf, 0x00, 0x00, 0x00, 0x0f, 0xcf, 0x00, 0x00, 
	0x00, 0x0f, 0xce, 0x00, 0x00, 0x00, 0x0f, 0xcc, 0x00, 0x00, 0x00, 0x0f, 0xcc, 0x00, 0x00, 0x00, 
	0x0f, 0xc8, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0x0f, 
	0xc0, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x0f, 0x00, 
	0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 
	0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
/* Constructors for the independent LEDs
*  These are: throbber, SAO1, SAO2, Power led, and Reset LED
* These LEDS are managed directly, not through the shift registers using a library for software managed PWM
*/

// SPIClass hspi(HSPI);
/*
bool setupSDCard(void)
{

    return SD.begin(SDCARD_CS);

}
*/
// Worker functions mostly for testing
//Prints all data from weather structure

void printForecast()
{
  // Create the structures that hold the retrieved weather
  OW_forecast  *forecast = new OW_forecast;

  Serial.print("\nRequesting weather information from OpenWeather... ");
//ow.getForecast(forecast, OPEN_WEATHER_MAP_APP_ID, latitude, longitude, units, OPEN_WEATHER_MAP_LANGUAGE);
//  ow.getForecast(forecast, api_key, latitude, longitude, units, language);
  Serial.println("Weather from OpenWeather\n");

  Serial.print("city_name           : "); Serial.println(forecast->city_name);
  Serial.print("sunrise             : "); Serial.println(forecast->sunrise);
  Serial.print("sunset              : "); Serial.println(forecast->sunset);
  Serial.print("Latitude            : "); Serial.println(ow.lat);
  Serial.print("Longitude           : "); Serial.println(ow.lon);
  Serial.print("Timezone            : "); Serial.println(forecast->timezone);
  Serial.println();

  if (forecast)
  {
    Serial.println("###############  Forecast weather  ###############\n");
    for (int i = 0; i < (MAX_DAYS * 8); i++)
    {
      Serial.print("3 hourly forecast   "); if (i < 10) Serial.print(" "); Serial.print(i);
      Serial.println();
      Serial.print("dt (time)        : "); Serial.print(forecast->dt[i]);

      Serial.print("temp             : "); Serial.println(forecast->temp[i]);
      Serial.print("temp.min         : "); Serial.println(forecast->temp_min[i]);
      Serial.print("temp.max         : "); Serial.println(forecast->temp_max[i]);

      Serial.print("pressure         : "); Serial.println(forecast->pressure[i]);
      Serial.print("sea_level        : "); Serial.println(forecast->sea_level[i]);
      Serial.print("grnd_level       : "); Serial.println(forecast->grnd_level[i]);
      Serial.print("humidity         : "); Serial.println(forecast->humidity[i]);

      Serial.print("clouds           : "); Serial.println(forecast->clouds_all[i]);
      Serial.print("wind_speed       : "); Serial.println(forecast->wind_speed[i]);
      Serial.print("wind_deg         : "); Serial.println(forecast->wind_deg[i]);
      Serial.print("wind_gust        : "); Serial.println(forecast->wind_gust[i]);

      Serial.print("visibility       : "); Serial.println(forecast->visibility[i]);
      Serial.print("pop              : "); Serial.println(forecast->pop[i]);
      Serial.println();

      Serial.print("dt_txt           : "); Serial.println(forecast->dt_txt[i]);
      Serial.print("id               : "); Serial.println(forecast->id[i]);
      Serial.print("main             : "); Serial.println(forecast->main[i]);
      Serial.print("description      : "); Serial.println(forecast->description[i]);
      Serial.print("icon             : "); Serial.println(forecast->icon[i]);

      Serial.println();
    }
  }
  // Delete to free up space and prevent fragmentation as strings change in length
  delete forecast;
}


//Converts wind direction in degrees to cardinal direction
String windDirection(int direction){
  direction %= 360;
  if(direction >= 349 && direction < 11) return F("N");
  else if(direction < 34) return F("NNE");
  else if(direction < 56) return F("NE");
  else if(direction < 79) return F("ENE");
  else if(direction < 101) return F("E");
  else if(direction < 124) return F("ESE");
  else if(direction < 146) return F("SE");
  else if(direction < 169) return F("SSE");
  else if(direction < 191) return F("S");
  else if(direction < 214) return F("SSW");
  else if(direction < 236) return F("SW");
  else if(direction < 259) return F("WSW");
  else if(direction < 281) return F("W");
  else if(direction < 304) return F("WNW");
  else if(direction < 326) return F("NW");
  return F("NNW");
}


String padWithZeroBelowTen(int d) {
  return d < 10 ? "0" + String(d) : String(d);
}
/*
String formatTime(time_t timestamp) {
  tm *date = gmtime(&timestamp);
  String year = "" + String(date->tm_year + 1900);
  String month = padWithZeroBelowTen(date->tm_mon + 1);
  String day = padWithZeroBelowTen(date->tm_mday);
  return year + "-" + month + "-" + day + " " + padWithZeroBelowTen(date->tm_hour) + ":" +
         padWithZeroBelowTen(date->tm_min) + ":" + padWithZeroBelowTen(date->tm_sec) + " UTC";
}
*/
void setup(){
    Serial.begin(115200);
//    Serial.println();
#ifdef DEBUG
    Serial.println("post-boot setup");
#endif    
//    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);

  updatePWM();
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  display.setFullWindow();
//  display.firstPage();
  display.fillScreen(GxEPD_WHITE);
  
// assign variables here for when we wake up
  timeToSleep = false; // we just woke up. So it's no time to sleep now! 
  ledRefreshDelay = BASELINELEDREFRESH; // reset for default led sequence delay
  readMem(); // get stored values

  sleepTimerVal = sleepTimeout * 60000 ;  // Timeout value is in minutes. Timer is in mseconds. 
  refreshTimer.start(REFRESHTIMERVAL);   // start a timer for display refresh
  sleepTimer.start(sleepTimerVal); // time until we for a sleep
  #ifdef DEBUG
  Serial.println("Sleep timer=" + String(sleepTimerVal));
  #endif
  //weather.key(openWeatherKey); //Inserting API key that allows us to use openweathermap.org API
  //weather.getWeatherByCityName(city_name,country,units,language); //Getting weather data from openweathermap.org
  //Execution time around 100ms usually
  //You can also use those commands to get weather:
  //getWeatherByID(ID,units,language);
  //weather.getWeatherByZIP(zipCode,countryCode,units,OPEN_WEATHER_MAP_LANGUAGE);
  //getWeatherByCoords(latitude,longitude,units,language);

  //status is http code of response
  /*
  if(weather.status == OW_OK) printWeather(weather);                                  //Data received, prints all data to Serial monitor
  else if(weather.status == OW_BAD_API_KEY) Serial.println(F("BAD API KEY!"));        //Wrong API key
  else if(weather.status == OW_CITY_NOT_FOUND) Serial.println(F("CITY NOT FOUND!"));  //City was not found
  else if(weather.status == OW_LIMIT_REACHED) Serial.println(F("REACHED 60 CALLS PER MINUTE LIMIT!")); //You have reached limit of your Free account (60 calls per minute)
  else if(weather.status == OW_SERVER_ERROR) Serial.println(F("SERVER ERROR!"));      //OpenWeather server error
 */
 
 // Setup the display
  updateStatic();
  updatePWM();



  
  timeIsReal = getTime();
// 74HC165 pins
  pinMode(load, OUTPUT);
  pinMode(clockEnablePin, OUTPUT);
  pinMode(clockIn, OUTPUT);
  pinMode(dataIn, INPUT);
 
// 74HC595 pins
  pinMode(latchPin,OUTPUT);
  pinMode(clockPin,OUTPUT);
  pinMode(dataPin,OUTPUT);
  //analogReadResolution(255);
  //analogSetAttenuation(ADC_2_5db);  //For all pins
  
  pinMode(vPin,INPUT);

#ifdef DEBUG
  Serial.println("setup done");
#endif
}
void printLocalTime(){
  struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
#ifdef DEBUG      
    Serial.println("Failed to obtain time");
#endif
    return;
  }
#ifdef DEBUG 
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
#endif  
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
}
bool getTime(){
    //disconnect WiFi as it's no longer needed
  if (initWiFi()){
    delay (250);
    getVoltage;
    showPartialUpdate("-ntp-");
    wifiStatus = YESWIFINOW;// show wifi connected 
    delay (250);
    // Initialize a NTPClient to get time
    timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
    timeClient.setTimeOffset(3600*TZ);
    timeClient.update();
    partialScreenBuffer = timeClient.getFormattedTime().substring(0,5);
#ifdef DEBUG
    Serial.print("Got time from server: ");
    Serial.println(partialScreenBuffer);
#endif
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    wifiStatus = NOWIFINOW; /// see if the icon changes
    showPartialUpdate(partialScreenBuffer);
    return true;
  }
  else{
    partialScreenBuffer = " N/A  ";
    wifiStatus = FAILWIFI; 
    showPartialUpdate(partialScreenBuffer);
    return false;
  };
}
bool initWiFi() {
  int failcnt = 0; 
  WiFi.mode(WIFI_STA);
#ifdef DEBUG
    Serial.print("Connecting to WiFi:"); 
    Serial.print(ssid); 
    Serial.print(" ");
    Serial.print("Connecting to scondary WiFi:"); 
    Serial.print(ssid2); 
#endif
  String ssidtemp = ssid;
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.print('.');
    delay(250);
    if (failcnt++ > 10){
        while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid2, password2);
        Serial.print(".");
        delay (250);
        if (failcnt++ > 10){   // try harder on #2
//          break;
          wifiStatus = FAILWIFI;
          ssid = ssidtemp; // put it back to normal for next time 
          return false;
        };
        ssid = ssid2;// set the primary to value of the backup for next time or for display. Reboot/EoProc resets this. 
      };
    };
  };
  Serial.println(WiFi.localIP());
  onlineReady = true; 
  wifiStatus = "b"; /// see if the icon changes
  return true;
}

void drawBattery() {
 
  uint16_t battx, batty, battw, batth;
//  battx = display.height()-20;
//  batty = display.width()-220;
  battx = 10;
  batty = display.height()-12;
  battw = 14;
  batth = 6;

  display.fillRect(battx-10,batty-10,battw+22,batth+15,GxEPD_WHITE); // Wipe out prior display. 

  
  int perc = round(percentage); 
//  if (volts >= 4.09){
  display.setCursor (battx,batty-4);
  display.setFont(&TomThumb);
//  display.setCursor (battx-4,batty);  
  display.println( String(volts) + " V");
//  }
//  else {
//  display.println(String(volts)+"v");
  display.drawRect(battx, batty-2, battw+3, batth+3,GxEPD_BLACK);
  display.fillRect(battx+2, batty, battw-1, batth-1,GxEPD_WHITE);
  display.fillRect(battx+battw+2,batty,3,5,GxEPD_BLACK); // the nub of the battery
#ifdef DEBUG
//  percentage = random(5,90);  /// for testing pick a random value
  Serial.printf("draw battery percent= %5.2f, volts= %5.3f , rounded= %u percent\n",percentage,volts,perc);
#endif
  display.fillRect(battx+2, batty,battw*(perc/100)-1, batth-1,GxEPD_BLACK); //fills in the battery bar
/*
  if (volts > 4.09){   // full or plugged in
    display.setCursor (battx-13,batty+8);  
//    display.setFont(&meteocons7pt7b);
//    display.setTextColor(GxEPD_BLACK);
//    display.print("G");
    display.setTextColor(GxEPD_BLACK); // be polite
    display.setFont(&data_latin8pt7b);
  };
  */
  if (percentage < 10){
    display.setCursor (battx+2,batty);  
    display.setFont(&data_latin8pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.print("!LOW!");
    display.setTextColor(GxEPD_BLACK); // be polite
  } 
}


void getVoltage(){

float rawloop, raw = 0;

for (int x=0 ; x<25 ; x++) {
  rawloop = analogRead(vPin);
#ifdef DEBUG
Serial.print(x);
#endif
  if (rawloop != 0) {
    raw = rawloop;
#ifdef DEBUG
    Serial.printf(".%3.1f.",raw);
#endif
    break;
  } else {
#ifdef DEBUG
Serial.print(" - ");
#endif
  delayMicroseconds(10);
  }
}  
  volts = (raw / 4096) * 4.1;  // Full run when charging voltage is 4.1 
    percentage = map(volts,3.3f,4.10f,0,100); // the unit runs down to about 3.1V
#ifdef DEBUG
// percentage = 47;   // for testing make it less than 100%
  Serial.printf("For pin %u Raw value is %5.3f, voltage is %5.3f and Battery Level is: %u \n",vPin,raw,volts,percentage);
#endif
}
bool valid(){
  int length = getResponse();
 // String yn = input;
  delay (250);
  Serial.println(lastName); Serial.println(String(lastName));
  Serial.printf("\nFN=%s LN=%s empno=%s SSID=%s PW=%s SSID2=%s PW2=%s\n",firstName,String(lastName),empNo,String(ssid),password,ssid2,password2);
//  Serial.printf("TZ=%i Longitude=%e Latitude=%e\n",TZ,longitude,latitude);
//  Serial.printf("TZ=%i ZipCode=%s \n",TZ,zipCode);
  Serial.println("\nDo you want this placed into memory?");
  input.trim(); 
  Serial.println("");
  Serial.println(input);
  input = input.substring(0,1);
  if (input == "y" || input == "Y"){
    return true;
  }
  else{
    Serial.println("Rejected. Try again.");
    return false;  
  };
}; // end validate 
void wakeUp(){

      delay (100);
      timeToSleep = false; // don't fall back asleep. 
      throbberled.Reset();
      resetled.Reset();
      powerled.Reset();
      SAO1led.Reset();
      SAO2led.Reset();

      updateStatic();
      partialScreenBuffer = "Awake!";
      showPartialUpdate(partialScreenBuffer);
      updatePWM();
      //Reprime the pump so to speak. 
      if (switchCheck(result)){
        processSwitches();
        writeLEDs();
      };
      timeIsReal = getTime();
#ifdef DEBUG
      Serial.println("tried to get time.");
#endif
      timeToSleep = false;
}
void takeASleepingPill(){
   
      partialScreenBuffer = "      ";
      showPartialUpdate(partialScreenBuffer);
//      display.nextPage();
      delay (100);
      ledResult = 0; //hard code to shut off the leds for the WriteLED call
      specialSwitchSet = false; // residual value in the logic of the switch checks
      result = 0; // Reset for new switch on startup the last result check
      writeLEDs();
//      updatePWM();
      delay (100);
      throbberled.Stop();
      resetled.Stop();
      powerled.Stop();
      SAO1led.Stop();
      SAO2led.Stop();
      ledResult = 0; //hard code to shut off the leds
      writeLEDs();
#ifdef DEBUG
Serial.println("This is the last gasp before sleep.  LEDS should be off!");
delay (500);
#endif
      while (!switchCheck(result)){
//        esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
//      display.powerdown();
//      esp_deep_sleep_start();
//        esp_light_sleep_start();
#ifdef DEBUG
//        Serial.println("woke from timer in takeapill");
#endif
      };
}
void updateMem(){
  if (prefs.begin("name")){
    Serial.print("Updating name...");
    prefs.clear();
    prefs.putString("FirstName",firstName);
    prefs.putString("Lastname",lastName);
//    loadPrefs.putUInt("Empno",empNo);
    prefs.putString("Empno",empNo);
    prefs.end(); // close for name
    Serial.print("Reading name...");
    prefs.getString("FirstName",firstName);
    prefs.getString("Lastname",lastName);
//    loadPrefs.getUInt("Empno",empNo);
    prefs.getString("Empno",empNo);
    
  }
  else{
    Serial.println("Error storing the name memory storage.");
  };
  prefs.end(); // close for name

  if (prefs.begin("credentials")){
    Serial.print("Updating credentials...");
    prefs.putString("SSID",ssid);
    prefs.putString("wifiPassword",password);
    prefs.putString("SSID2",ssid2);
    prefs.putString("wifiPassword2",String(password2));
    prefs.putInt("TZ",TZ);
    prefs.putString("zipCode",zipCode);
    prefs.putString("countryCode",countryCode);
    prefs.putString("openWeatherKey",openWeatherKey);  
    prefs.putInt("sleepTimeout",sleepTimeout);  // in minutes
    
    
    Serial.print("Reading credentials...");
   
   
    prefs.getString("SSID",String(ssid));
    prefs.getString("wifiPassword",String(password));
    prefs.getString("SSID2",String(ssid2));
    prefs.getString("wifiPassword2",String(password2));
    prefs.getInt("TZ",TZ);
    prefs.getString("zipCode",zipCode);
    prefs.getString("countryCode",countryCode);
    prefs.getString("openWeatherKey",openWeatherKey);
    prefs.getInt("sleepTimeout",sleepTimeout);
  
    prefs.end(); // close for credentials
    Serial.printf("FN=%s LN=%s empno=%s SSID=%s PW=%s SSID2=%s PW2=%s\n",firstName,lastName,empNo,ssid,password,ssid2,password2);
    Serial.printf("TZ=%i Zip code=%s,Country=%s,Open Weather Key=%s, Sleep timeout=%u",TZ,zipCode,countryCode,openWeatherKey, sleepTimeout);  // in minutes
  }
  else{
    Serial.println("Error storing the credentials memory storage.");
  };
  prefs.end();
};

int getResponse() {
  // check if data is available
  int rlen = 0;
  Serial.setTimeout(SERIALTIMEOUT);
  while (Serial.available() == 0) {}     //wait for data available
  input = Serial.readString();  //read until timeout
  input.trim();
  return input.length();
  Serial.println(input) ;                       
}

void getName(){
  int len = 0;
  Serial.setTimeout(SERIALTIMEOUT);
  delay (2000);   // pause to get to the serial input screen 
  Serial.flush();
  readMem();
 do {
  Serial.printf("\nFN=%s LN=%s empno=%s SSID=%s PW=%s SSID2=%s PW2=%s\n",firstName,lastName,empNo,ssid,password,ssid2,password2);
  Serial.printf("TZ=%i Zip Code=%s Country=%s Open Weather Key=%s, Sleep Timeout=%u\n",TZ,zipCode,countryCode,openWeatherKey,sleepTimeout);
  Serial.println("\nDo you want this placed into memory?");
  if (valid()){
    updateMem();
    break;
  };
    Serial.print("\nEnter first name:");
    len = getResponse();
    if (len < 1) return;
    firstName = input;
    firstName.trim(); 
    Serial.println(firstName);  
    delay (250);
    Serial.print("Enter last name:"); 
    len = getResponse();
    if (len < 1) return;
    lastName=input;
    lastName.trim();
    lastName.substring(0,11);
    Serial.println(lastName);
    Serial.println();
    delay(250);
    Serial.print("Employee num (any value):"); 
    len = getResponse();
    if (len < 1) return;
  //empno = input.toInt();
    empNo = input;
    if (len > 8)
      empNo.substring(0,8);
    Serial.println(empNo);
    delay (250);
    Serial.print("Enter Primary SSID:");
    len = getResponse();
    if (len < 1) {
      input = "None";
    };
    ssid=String(input);
    ssid.trim();
    Serial.println(ssid);
    delay (250);

    Serial.print("Enter Primary WiFi Password:");
    len = getResponse();
    if (len < 1) {
      input = "";
    };
    password=input;
 // wifiPassword=String((char *)input);
    password.trim();
    Serial.println(password);
    Serial.println();
    delay (250);
    Serial.print("Enter backup SSID:");
    len = getResponse();
    if (len < 1) {
      input = "None";
    };
    ssid2=String(input);
    ssid2.trim();
    Serial.println(ssid2);
    delay (250);
    Serial.print("Enter backup WiFi Password:");
      len = getResponse();
    if (len < 1) {
      input = "";
    };
    password2=input;
 // wifiPassword=String((char *)input);
    password2.trim();
    Serial.println(password2);
    delay(250);
    Serial.print("Enter TZ (GMT offset in hours) Default is -5 Eastern US:");
    len = getResponse();
    if (len < 1) {
      TZ=-5;
    }
    else{
      TZ=input.toInt();
    }
    Serial.println(TZ);
    delay (250);
    Serial.print("Enter weather location ZIP CODE");
    len = getResponse();
    if (len < 1) {
      zipCode = "49633";
    }
    else{
      input.trim();
      zipCode=input;
    };
    Serial.println(zipCode);
    delay (250);  
    Serial.print("Enter weather location country code (two characters - e.g. US)");
    len = getResponse();
    if (len < 1) {
      countryCode = "US";
    }
    else{
      input.trim();
      countryCode=input;
    };
    Serial.println(countryCode);

  Serial.print("Enter the Open Weather Forecast key (https://openweathermap.org)");
    len = getResponse();
    if (len < 1) {
      openWeatherKey = "ec5f1053c471c0d461c4b2ab3e75745e";
    }
    else{
      openWeatherKey=String(input);
      openWeatherKey.trim();
    };
    Serial.println(openWeatherKey);
    delay (250);  
    Serial.print("Enter Sleep timer in minutes (default 60 minutes)");
    len = getResponse();
    if ((len < 1) || (len > 24)) {
      sleepTimeout = 60;
    }
    else{
      input.trim();
      sleepTimeout=input.toInt();
    };
    Serial.println(countryCode);

  } while (true); // end do loop
  return;
};

void inputPrefs(){
    delay (500);  
    readMem();
    getName();
    readMem();
    delay (4000);
};

void readMem(){
  if (prefs.begin("name",true)){
    firstName = prefs.getString("FirstName");
    firstName = firstName.substring(0,11);
    lastName = prefs.getString("Lastname");
    lastName = lastName.substring(0,20);
//    empNo = prefs.getUInt("Empno");
    empNo = prefs.getString("Empno");
    empNo = empNo.substring(0,7);
        prefs.end(); // close for name
  }
  else{
    Serial.println("Error retrieving the name memory storage.");
  };
  if (prefs.begin("credentials",true)){
    ssid = prefs.getString("SSID");
    password = prefs.getString("wifiPassword");
    ssid2 = prefs.getString("SSID2");
    password2 = prefs.getString("wifiPassword2");
    gmtOffset_hours = prefs.getInt("TZ");
    zipCode = prefs.getString("zipCode");
    countryCode = prefs.getString("countryCode");
    openWeatherKey = prefs.getString("openWeatherKey");
    sleepTimeout = prefs.getInt ("sleepTimeout");
    prefs.end(); // close for credentials
  }
  else{
    Serial.println("Error retreiving the credentials memory storage.");
  };
};
void updatePWM(){
    throbberled.Update();
    resetled.Update();
    powerled.Update();
    SAO1led.Update();
    SAO2led.Update();
//    boardled.Update();
};
void showPartialUpdate(String inputString){

// Lilygo t5 2.3 is 212 x 104 
// YSTATUS is 128    XSTATUS is 235

    uint16_t wifi_x =  223 , wifi_y = YSTATUS-3;
    uint16_t icon_x = wifi_x - 22, icon_y = YSTATUS-3;
    uint16_t message_x = wifi_x - 65, message_y = YSTATUS-3;
    display.setTextColor(GxEPD_BLACK);

#ifdef DEBUG
    Serial.printf("width=%u , height=%u\n ",display.width(),display.height());
    Serial.printf("iwidth=%u , iheight=%u\n",icon_x,icon_y);
    Serial.printf("mwidth=%u , mheight=%u\n",message_x,message_y);
    Serial.printf("wwidth=%u , wheight=%u\n",wifi_x,wifi_y);
#endif
    display.fillRect(XSTATUS-86, YSTATUS-25,93,25, GxEPD_WHITE);
    display.setFont(&WIFI10pt7b);
    display.setCursor(wifi_x,wifi_y);
if (timeToSleep){
    display.setFont(&WIFI10pt7b);
    wifiStatus = NOWIFINOW; /// see if the icon changes
  }; 
// show wifi status      
    display.print(wifiStatus);
    display.setCursor(icon_x,icon_y);
    if (timeToSleep){
      display.setFont(&meteocons12pt7b);
      display.print("C");  // sleepy icon
    }
    else{
      display.setFont(&data_latin10pt7b);
      display.print(" ");
    };   
    display.setCursor(message_x,message_y);
    display.print(inputString); // should hold the time string or icons
    display.setFont(&data_latin10pt7b); // set back to normal
    drawBattery();
// Last is to display the icon 
    display.nextPage();
}
void updateStatic(){
    display.setRotation(1);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
//    display.setPartialWindow(0, 0, display.width(), display.height());
    display.setFont(&FreeMono18pt7b);
    display.drawBitmap(0,15,epd_bitmap_Tandem_Chevron_inverted,34,34,GxEPD_BLACK);
// first name size depends on its length
//     display.update();
    display.setCursor(45,66);
    if(firstName.length() < 6){
      Serial.println("< 7 char fname");
      display.setFont(&data_latin42pt7b);
      display.setCursor(45,66);

    }else
    if (firstName.length() < 8){
      display.setFont(&data_latin28pt7b);
//    display.setFont(&FreeMono24pt7b);
      display.setCursor(45,50);
//     Serial.println("7 char fname");
    }else
    if (firstName.length() < 9){
//        Serial.println("8 char fname");
        display.setFont(&data_latin24pt7b);
        display.setCursor(45,40);
    }else
    if (firstName.length() < 11){
//          Serial.println("10 char fname");
      display.setFont(&data_latin20pt7b);  
      display.setCursor(45,40);
    }else
    if (firstName.length() < 12){
      display.setFont(&data_latin18pt7b);
      display.setCursor(45,40);

    }
    else
    { // must be longer than 11 
      display.setCursor(45,40);
      display.setFont(&data_latin12pt7b);
    };
    display.println(firstName) ; 
    display.setCursor(45,98);
    display.setFont(&data_latin18pt7b);
    if((firstName.length() < 13) && (firstName.length() > 7 )){
      display.setCursor(45,75);
    };
    if(lastName.length() > 11){
      display.setFont(&data_latin12pt7b);
            display.setCursor(2,90); // shift over to the left for long names
    };
    display.println(lastName);
  //  display.update();

    display.setFont(&data_latin10pt7b);
    display.setCursor(46,YSTATUS-3);
    display.print("#"+ String(empNo)) ;
    getVoltage();
    drawBattery();
    display.setFont(&TomThumb);
    display.setRotation(0);   // rotated sideways
    display.setCursor(4,250);  // trying to find the right place for this
    display.print(DISVERSION);
    display.print(" ");
    display.print(String(sleepTimeout));
    display.print(" ");
    display.print(String(ssid));
  /*
    display.print("/");
    display.print(String(ssid2));
*/
    display.setRotation(1);    // horizontal as normal
    display.setFont(&data_latin10pt7b);  // be polite for next use

    display.nextPage(); // display the page 
    int pg = display.pages();
#ifdef DEBUG
    Serial.printf("Number of pages=%u\n",pg);
#endif
    delay (100); 
    showPartialUpdate(partialScreenBuffer);
};
void ledSetup(int which){
  switch (which) {
    case 0b000:   // cylon inverse
    ledArray[0] = 65535;  //   B1111 1111 1111 1111;
    ledArray[1] = 32766;  //   B0111 1111 1111 1110;
    ledArray[2] = 49149;  //   B1011 1111 1111 1101;
    ledArray[3] = 59339;  //   B1101 1111 1111 1011;
    ledArray[4] = 61431;  //   B1110 1111 1111 0111;
    ledArray[5] = 63471;  //   B1111 0111 1110 1111;
    ledArray[6] = 64479;  //   B1111 1011 1101 1111;
    ledArray[7] = 64959;  //   B1111 1101 1011 1111;
    ledArray[8] = 65151;  //   B1111 1110 0111 1111;
    ledArray[9] = 64959;  //   B1111 1101 1011 1111;
    ledArray[10] = 64479; //   B1111 1011 1101 1111;
    ledArray[11] = 63471; //   B1111 0111 1110 1111;
    ledArray[12] = 61431; //   B1110 1111 1111 0111;
    ledArray[13] = 59339; //   B1101 1111 1111 1011;
    ledArray[14] = 49149; //   B1011 1111 1111 1101;
    ledArray[15] = 32766; //   B0111 1111 1111 1110;
    
    break;
    case 0b001:   // countup (cumulative on)
    ledArray[0] =  1 ;     //  B0000 0000 0000 0001;
    ledArray[1] =  3 ;     //  B0000 0000 0000 0011;
    ledArray[2] =  7 ;     //  B0000 0000 0000 0111;
    ledArray[3] =  15;     //  B0000 0000 0000 1111;
    ledArray[4] =  31;     //  B0000 0000 0001 1111;
    ledArray[5] =  63;     //  B0000 0000 0011 1111;
    ledArray[6] =  127;    //  B0000 0000 0111 1111;
    ledArray[7] =  255;    //  B0000 0000 1111 1111;
    ledArray[8] =  511;    //  B0000 0001 1111 1111;
    ledArray[9] =  1023;   //  B0000 0011 1111 1111;
    ledArray[10] = 2047;   //  B0000 0111 1111 1111;
    ledArray[11] = 4095;   //  B0000 1111 1111 1111;
    ledArray[12] = 8191;   //  B0001 1111 1111 1111;
    ledArray[13] = 16383;  //  B0011 1111 1111 1111;
    ledArray[14] = 32767;  //  B0111 1111 1111 1111;
    ledArray[15] = 65535;  //  B1111 1111 1111 1111;
    break;
    case 0b010:   // slide over could ust use <<1 instead
    ledArray[0] = 32767;  //   B0111 1111 1111 1111;
    ledArray[1] = 65534;  //   B1111 1111 1111 1110;
    ledArray[2] = 65533;  //   B1111 1111 1111 1101;
    ledArray[3] = 65531;  //   B1111 1111 1111 1011;
    ledArray[4] = 65527;  //   B1111 1111 1111 0111;
    ledArray[5] = 65519;  //   B1111 1111 1110 1111;
    ledArray[6] = 65503;  //   B1111 1111 1101 1111;
    ledArray[7] = 65471;  //   B1111 1111 1011 1111;
    ledArray[8] = 65407;  //   B1111 1111 0111 1111;
    ledArray[9] = 65279;  //   B1111 1110 1111 1111;
    ledArray[10] = 65023; //   B1111 1101 1111 1111;
    ledArray[11] = 64411; //   B1111 1011 1111 1111;
    ledArray[12] = 63487; //   B1111 0111 1111 1111;
    ledArray[13] = 61439; //   B1110 1111 1111 1111;
    ledArray[14] = 57343; //   B1101 1111 1111 1111;
    ledArray[15] = 49151; //   B1011 1111 1111 1111;
    break;
 case 0b011:   // slide on down (off)
    ledArray[0] = 32768;  //   B1000 0000 0000 0000;
    ledArray[1] = 16384;  //   B0100 0000 0000 0000;
    ledArray[2] = 8192;   //   B0010 0000 0000 0000;
    ledArray[3] = 4096;   //   B0001 0000 0000 0000;
    ledArray[4] = 2048;   //   B0000 1000 0000 0000;
    ledArray[5] = 1024;   //   B0000 0100 0000 0000;
    ledArray[6] = 512;    //   B0000 0010 0000 0000;
    ledArray[7] = 256;    //   B0000 0001 0000 0000;
    ledArray[8] = 128;    //   B0000 0000 1000 0000;
    ledArray[9] = 64;     //   B0000 0000 0100 0000;
    ledArray[10] = 32;    //   B0000 0000 0010 0000;
    ledArray[11] = 16;    //   B0000 0000 0001 0000;
    ledArray[12] = 8;     //   B0000 0000 0000 1000;
    ledArray[13] = 4;     //   B0000 0000 0000 0100;
    ledArray[14] = 2;     //   B0000 0000 0000 0010;
    ledArray[15] =  1;    //   B0000 0000 0000 0001;
    break;
 case 0b100:   // slide around
    ledArray[0] = 32768;  //   B1000 0000 0000 0000;
    ledArray[1] = 1;      //   B0000 0000 0000 0001;
    ledArray[2] = 16384;  //   B0100 0000 0000 0000;
    ledArray[3] = 2;      //   B0000 0000 0000 0010;
    ledArray[4] = 8192;   //   B0010 0000 0000 0000;
    ledArray[5] = 4;      //   B0000 0000 0000 0100;
    ledArray[6] = 4096;   //   B0001 0000 0000 0000;
    ledArray[7] = 8;      //   B0000 0000 0000 1000;
    ledArray[8] = 2048;   //   B0000 1000 0000 0000;
    ledArray[9] = 16;     //   B0000 0000 0001 0000;
    ledArray[10] = 1024;  //   B0000 0100 0000 0000;
    ledArray[11] = 32;    //   B0000 0000 0010 0000;
    ledArray[12] = 512;   //   B0000 0010 0000 0000;
    ledArray[13] = 64;    //   B0000 0000 0100 0000;
    ledArray[14] = 256;   //   B0000 0001 0000 0000;
    ledArray[15] = 128;   //   B0000 0000 1000 0000;

    break;
 case 0b101:   // slide on down (on)
    ledArray[0] = 32767;  //   B0111 1111 1111 1111;
    ledArray[1] = 65534;  //   B1111 1111 1111 1110;
    ledArray[2] = 65533;  //   B1111 1111 1111 1101;
    ledArray[3] = 65531;  //   B1111 1111 1111 1011;
    ledArray[4] = 65527;  //   B1111 1111 1111 0111;
    ledArray[5] = 65519;  //   B1111 1111 1110 1111;
    ledArray[6] = 65503;  //   B1111 1111 1101 1111;
    ledArray[7] = 65471;  //   B1111 1111 1011 1111;
    ledArray[8] = 65407;  //   B1111 1111 0111 1111;
    ledArray[9] = 65279;  //   B1111 1110 1111 1111;
    ledArray[10] = 65023; //   B1111 1101 1111 1111;
    ledArray[11] = 64411; //   B1111 1011 1111 1111;
    ledArray[12] = 63487; //   B1111 0111 1111 1111;
    ledArray[13] = 61439; //   B1110 1111 1111 1111;
    ledArray[14] = 57343; //   B1101 1111 1111 1111;
    ledArray[15] = 49151; //   B1011 1111 1111 1111;

    break;
 case 0b110:   // count up (off)
    ledArray[0] =  0B1000000000000000;
    ledArray[1] =  0B1100000000000000;
    ledArray[2] =  0B1110000000000000;
    ledArray[3] =  0B1111000000000000;
    ledArray[4] =  0B1111100000000000;
    ledArray[5] =  0B1111110000000000;
    ledArray[6] =  0B1111111000000000;
    ledArray[7] =  0B1111111100000000;
    ledArray[8] =  0B1111111110000000;
    ledArray[9] =  0B1111111111000000;
    ledArray[10] = 0B1111111111100000;
    ledArray[11] = 0B1111111111110000;
    ledArray[12] = 0B1111111111111000;
    ledArray[13] = 0B1111111111111100;
    ledArray[14] = 0B1111111111111110;
    ledArray[15] = 0B1111111111111111;

    break;
 case 0b111:   // cylon on
    ledArray[0] =  0B1000000000000000;  
    ledArray[1] =  0B1110000000000000;
    ledArray[2] =  0B0111000000000000;
    ledArray[3] =  0B0011100000000000;
    ledArray[4] =  0B0001110000000000;
    ledArray[5] =  0B0000111000000000;
    ledArray[6] =  0B0000011100000000;
    ledArray[7] =  0B0000001110000000;
    ledArray[8] =  0B0000000111000000;
    ledArray[9] =  0B0000000011100000;
    ledArray[10] = 0B0000000001110000;
    ledArray[11] = 0B0000000000111000;
    ledArray[12] = 0B0000000000011100;
    ledArray[13] = 0B0000000000001110;
    ledArray[14] = 0B0000000000000111;
    ledArray[15] = 0B0000000000000011;
    break;
  default:
    // all off 
    ledArray[0] =  0B0000000000000000;  
    ledArray[1] =  0B0000000000000000;
    ledArray[2] =  0B0000000000000000;
    ledArray[3] =  0B0000000000000000;
    ledArray[4] =  0B0000000000000000;
    ledArray[5] =  0B0000000000000000;
    ledArray[6] =  0B0000000000000000;
    ledArray[7] =  0B0000000000000000;
    ledArray[8] =  0B0000000000000000;
    ledArray[9] =  0B0000000000000000;
    ledArray[10] = 0B0000000000000000;
    ledArray[11] = 0B0000000000000000;
    ledArray[12] = 0B0000000000000000;
    ledArray[13] = 0B0000000000000000;
    ledArray[14] = 0B0000000000000000;
    ledArray[15] = 0B0000000000000000;
    break;
  };
}; // end of LEDSetup 

  void writeLEDs() 
  {
  // Write to LEDs
   // ST_CP LOW to keep LEDs from changing while reading serial data

      digitalWrite(latchPin, LOW); 
    // Shift out the bits
    // if the switches match the LEDs...
//    if (result != UNIVERSALNUMBER){
      if (!specialSwitchSet){
        ledRefreshDelay = BASELINELEDREFRESH;   // reset if this was change in a prior iteration
        shiftOut(dataPin,clockPin,MSBFIRST,(ledResult >> 8));   
        shiftOut(dataPin,clockPin,MSBFIRST,ledResult);
        digitalWrite(latchPin, HIGH);
        delay(50); 
    }
    else {   // then there is a loop display
        if (ledCounter > 15 ){ // dont overflow
            ledCounter = 14;
            if (noReverse){
              ledCounter = 0; // set back to beginning instead of going backwards
              flipflop = true; // should already be true
            }
            else
              flipflop = false;
          }
          else{
            if (ledCounter < 0){
              flipflop = true;
              ledCounter = 0;  // just in case we underflow 
            };
          };

//#ifdef DEBUG
//  print_bytes(ledArray[ledCounter]);
//  Serial.println(ledCounter);
//#endif

        shiftOut(dataPin,clockPin,MSBFIRST,(ledArray[ledCounter] >> 8));   
        shiftOut(dataPin,clockPin,MSBFIRST,ledArray[ledCounter]);
        digitalWrite(latchPin, HIGH);
        delay(50);
        if (flipflop) {
            ++ledCounter;
        }
        else{ 
            --ledCounter;
        };
        updatePWM();
        delay(ledRefreshDelay); // set this variable to ms for LED to display.
      };
  };

// Function to reverse bits in a given range [start, end]
uint16_t reverseBits(uint16_t num) {

  // Reverse the extracted bits
  uint16_t reversedNum = 0;
  int bitPosition = 15; 

  while (num > 0){
    uint16_t bit = num & 1;
    reversedNum |= (bit << bitPosition);
    num >>= 1;
    bitPosition--;
  }
    // Put the reversed bits into response
  return reversedNum;
};
void print_bytes(uint16_t number) { 
  byte i; 
//  Serial.println("*Shift Register Values:*\r\n");

  for(byte i=0; i<=DATA_WIDTH-1; i++) 
  { 
    Serial.print("P");
    Serial.print(i+1);
    Serial.print(" "); 
  }
  Serial.println();
  for(byte i=0; i<=DATA_WIDTH-1; i++) 
  { 
    Serial.print(number >> i & 1, BIN); 
    
    if(i>8){Serial.print(" ");}
    Serial.print("  ");    
  } 
  Serial.print("\n"); 

};// End Print Byte

int switchCheck(uint16_t bytesVal) {
// Read Switches      
// Write pulse to load pin
// PL (load) pin 14  
// CE (clockEnablePin) pin 15       
// Q7 (dataIn) pin 34  
// CP (clockIn)  pin 2  
  uint16_t bitVal;
  bytesVal = 0;
 
// Get data from 74HC165

    digitalWrite(clockEnablePin, HIGH);
    digitalWrite(load, LOW);
    delayMicroseconds(5);
    digitalWrite(load, HIGH);
    digitalWrite(clockEnablePin, LOW);

    for(int i = 0; i < DATA_WIDTH; i++)
    {
        bitVal = digitalRead(dataIn);

        bytesVal |= (bitVal << ((DATA_WIDTH-1) - i));
        digitalWrite(clockIn, HIGH);
        delayMicroseconds(5);
        digitalWrite(clockIn, LOW);
    };
  result = bytesVal ^ 0b1111111100000000;    // this is a kludge to fix high order HC74165 chip compliment value

if (result == lastResult) { 
    return false;
  }
  else{
    lastResult = result;
    specialSwitchSet = false;
    noReverse = false; // reset so the the loops go both ways
#ifdef DEBUG
    Serial.println("New switch value!");
// Print to serial monitor
  Serial.print("Switch pin states:\r\n");
  print_bytes(result);
#endif
    sleepTimer.restart();  // the sleep time resets each time a switch is pressed
    return true;
  };

}; // end of switchCheck

  void processSwitches(){
  int which;
//  Serial.println(result,BIN);
  if (result == SLEEPSWITCHEDON){
    timeToSleep = true;
    ledResult = 0; //hard code to shut off the leds
    writeLEDs();
    };
  if (result == UNIVERSALNUMBER) { 
    which = random(0,7);
    ledSetup(which);
    specialSwitchSet = true;
#ifdef DEBUG
Serial.println("Universal number");
#endif
  };
  if (result == CYLON) { 
    which = 0B111;
    ledSetup(which);
    ledRefreshDelay = 25;  // slower for cylon
    specialSwitchSet = true;
#ifdef DEBUG
Serial.println("Cylon");
#endif
  };
  if (result == LOADREGISTERS) { 
    which = 0B110;
    ledSetup(which);
    ledRefreshDelay = 0750;  // slower for load registers
    noReverse = true;
    specialSwitchSet = true;
#ifdef DEBUG
Serial.println("register display");
#endif
  };

  if (result == GETNEWPREFS) { 
      inputPrefs();
//      Serial.println("TURN OFF THE SWITCHES NOW!");
      delay (5000);
  };


  ledResult = reverseBits(result); //This sets up the proper order for the physical switches 
#ifdef DEBUG
  Serial.println(ledResult,BIN);
#endif
  };

  void loop()
{
    currentMill = millis();
    currentMillSinceLastUpdate = ((currentMill-lastUpdate)%1000);  // time to go get a new time
    currentSeconds = lastNTPTime + ((currentMill - lastUpdate)/1000);
    if (currentMill - lastUpdate >= updateInterval){
      lastUpdate = currentMill;
      timeIsReal = getTime();
      getVoltage();
    }
    else if (currentMill - lastIntervalUpdate >= displayInterval){   // time to refresh display 
        getVoltage();
        lastIntervalUpdate = currentMill; // reset interval 
        if (timeIsReal){
        String minuteStr = String((timeClient.getMinutes() < 10) ? "0" + String(timeClient.getMinutes()) : String(timeClient.getMinutes())); // do not suppress zero
        partialScreenBuffer = String(timeClient.getHours()) + ":" + minuteStr; 
        }
        else partialScreenBuffer = " N/A ";
#ifdef DEBUG 
        Serial.println(partialScreenBuffer); 
#endif 
        showPartialUpdate(partialScreenBuffer);
    };
    updatePWM();
    if (switchCheck(result)){
      processSwitches();
#ifdef DEBUG
    Serial.println("New switch value!");
// Print to serial monitor
  Serial.print("Switch pin states:\r\n");
  print_bytes(result);
#endif
    };
    writeLEDs(); // this procedure will look to see if it is array or standard value
    if (refreshTimer.available()) {
      refreshTimer.stop();
      showPartialUpdate(partialScreenBuffer);
      refreshTimer.start(REFRESHTIMERVAL);// Every minute
  }
  if (sleepTimer.available()) {
      sleepTimer.stop(); 
      sleepTimer.start(sleepTimerVal);// Every sleep timer timout val
        Serial.println("Sleep timer=" + String(sleepTimerVal));
      timeToSleep = true; 
  #ifdef DEBUG
    Serial.println("Sleep timer popped. time to sleep..");
  #endif
  }
  if (timeToSleep){
#ifdef DEBUG
    Serial.println("Going to sleep now.");
#endif
    takeASleepingPill();
#ifdef DEBUG
    Serial.println("Wakey wakey.");
#endif
    wakeUp();
    };
};
