#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiSTA.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiType.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
//#include <ESP8266WiFi.h>


#include <Timers.h>
#include <jled.h>

#include <ESPHTTPClient.h>
#include <time.h>
#include "SunMoonCalc.h"


/*
First version of the badge - preferences must be loaded separately using the name oader tool. 
M. Keith Moore
*/
//#define DEBUG TRUE // Comment this out for non-debug
#define ENABLE_GxEPD2_GFX 0
#define DISVERSION "V3c"
#define XSTATUS 235  // beginning of the status @ this column
#define YSTATUS 128  // beginning of the status at this row
#define DATA_WIDTH 16
#define uS_TO_S_FACTOR 1000000 // Conversion factor for micro seconds to seconds 
#define TIME_TO_SLEEP 300 // Time ESP32 will go to sleep (in seconds) 
// 2 minutes
#define SERIALTIMEOUT 1000
#define BASELINELEDREFRESH 100
#define REFRESHTIMERVAL 60000  // 60 seconds 
//#define UNIVERSALNUMBER 0B0000000000101010 // normally this should be 42. switches set to this turns on the blinkies
#define UNIVERSALNUMBER   0B1110000000000000 // this is a test value 7 (the are little endian left side- forced here to right side) 
#define CYLON   0B1010000000000000 // this is a test value 5 (the are little endian left side- forced here to right side in the code) 
#define LOADREGISTERS   0B1100000000000000 // this is a test value 3 (the are little endian left side- forced here to right side in the code) 
#define SLEEPSWITCHEDON 0B0000000000000111 // low value 7 makes me sleepy. 

#define LILYGO_T5_V213

#include <Preferences.h>
#include <boards.h>
#include <GxEPD.h>
#include <GxEPD2_BW.h>
//#include <GxEPD2_3C.h>
//#include <GxEPD2_4C.h>
//#include <GxEPD2_7C.h>
//#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w  form DKE GROUP
#define GxEPD2_DRIVER_CLASS GxEPD2_213_BN // DEPG0213BN  122x250, SSD1680, (FPC-7528B), TTGO T5 V2.4.1, V2.3.1
//#define GxEPD2_DRIVER_CLASS GxEPD2_213_GDEY0213B74 // GDEY0213B74 122x250, SSD1680, (FPC-A002 20.04.08)
//#include <SD.h>
#include <FS.h>
#include <WiFi.h>

//#include GxEPD_BitmapExamples
// FreeFonts from Adafruit_GFX
// Fonts are located in the libraries/fonts directory under the Adafruit_GFX_Library folder in the fontss folder. 

#include <Fonts/data_latin6pt7b.h>
#include <Fonts/data_latin8pt7b.h>
#include <Fonts/data_latin10pt7b.h>
#include <Fonts/data_latin12pt7b.h>
#include <Fonts/data_latin18pt7b.h>
#include <Fonts/data_latin20pt7b.h>
#include <Fonts/data_latin22pt7b.h>
#include <Fonts/data_latin24pt7b.h>
#include <Fonts/data_latin28pt7b.h>
#include <Fonts/data_latin30pt7b.h>
#include <Fonts/data_latin32pt7b.h>
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
#include <Fonts/WIFI8pt7b.h>
#include <Fonts/WIFI10pt7b.h>
#include <Fonts/WIFI12pt7b.h>
// 'wifi_PNG62355', 20x14px
// 'wifi_PNG62355', 20x14px
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


#include <GxIO/GxIO_SPI/GxIO_SPI.h>
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
// Define Connections to 74HC595
 
// ST_CP pin 12
const int latchPin = 27;
// SH_CP pin 11
const int clockPin = 12;
// DS pin 14
const int dataPin = 0; 


 // Define data array
uint16_t ledArray[16];
int ledCounter = 0;  // used to hold the current array element 
int ledRefreshDelay = 100; // milliseconds of led time shown
uint16_t ledResult = 0;
bool flipflop = true;  // used to reversing the LED displays 

const int vPin = 35;  // was 13 but the board shows pin 35 as battery test? 
const int throbberPin = 13;  // chevron pin 
const int SAO1Pin = 25;  // SAO pin
const int SAO2Pin = 26;  //  SAO pin
const int resetPin = 32; // Reset LED pin
const int powerPin = 33; // power LED pin

//GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
//GxEPD_Class display(io, EPD_RSET, EPD_BUSY);
GxEPD2_BW<GxEPD2_290_GDEY029T94, GxEPD2_290_GDEY029T94::HEIGHT>
display(GxEPD2_290_GDEY029T94(/*CS=5*/ EPD_CS, /*DC=*/ EPD_DC, /*RST=*/ EPD_RSET, /*BUSY=*/ EPD_BUSY)); // GDEY029T94  128x296, SSD1680, (FPC-A005 20.06.15)


// Setup software PWM for leds. 

auto throbberled = JLed(throbberPin).Breathe(4500).DelayAfter(1000).Forever();
//auto throbberled = JLed(throbberPin).Candle(5 /* speed */, 150 /*jitter*/).Forever();
//auto resetled = JLed(resetPin).Breathe(3000).MinBrightness(10).DelayAfter(500).Repeat(50);
//auto powerled = JLed(powerPin).Breathe(2500).MinBrightness(10).DelayAfter(500).Repeat(50);
auto resetled = JLed(resetPin).Breathe(2000).MaxBrightness(30).MinBrightness(5).DelayAfter(500).Forever();
auto powerled = JLed(powerPin).Breathe(3000).MaxBrightness(30).MinBrightness(5).DelayAfter(500).Forever();
auto SAO1led = JLed(SAO1Pin).Candle(5 /* speed */, 100 /*jitter*/).Forever();
auto SAO2led = JLed(SAO2Pin).Candle(1 /* speed */, 500 /*jitter*/).Forever();

// Setup timer(s)

Timers refreshTimer;
// Wifi Time (NTP setup)

//const char* ntpServer = "pool.ntp.org";
const char* ntpServer = "time.nist.gov";
const long  gmtOffset_sec = -5;
const int   daylightOffset_sec = 3600;


//

String firstName;
String lastName;
String password;
String ssid;
String input;
String partialScreenBuffer = "                ";
String wifiStatus = "a";  // this will be the WIFI font character representing the current Wifi state (a, b, c, etc.) 
String empNo;
float volts;
uint16_t result, lastResult = 0;
// Set up to use flash memory preference library
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

SPIClass hspi(HSPI);
/*
bool setupSDCard(void)
{

    return SD.begin(SDCARD_CS);

}
*/


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
void setup(void)
{
    Serial.begin(115200);
//    Serial.println();
//    Serial.println("setup");
    
    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
/*
#if defined(LILYGO_EPD_DISPLAY_102)
    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);
#endif //LILYGO_EPD_DISPLAY_102
*/
/*
#if defined(LILYGO_T5_V102)
    pinMode(POWER_ENABLE, OUTPUT);
    digitalWrite(POWER_ENABLE, HIGH);
#endif //  LILYGO_T5_V102
*/
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  display.setFullWindow();
  display.firstPage();
  display.fillScreen(GxEPD_WHITE);
  
  pinMode(vPin,INPUT);

// 74HC165 pins
  pinMode(load, OUTPUT);
  pinMode(clockEnablePin, OUTPUT);
  pinMode(clockIn, OUTPUT);
  pinMode(dataIn, INPUT);
 
// 74HC595 pins
  pinMode(latchPin,OUTPUT);
  pinMode(clockPin,OUTPUT);
  pinMode(dataPin,OUTPUT);


// assign variables here for when we wake up
  timeToSleep = false; // we just wok up. So it's no time to sleep now! 
  ledRefreshDelay = BASELINELEDREFRESH; // reset for default led sequence delay

  refreshTimer.start(REFRESHTIMERVAL);   // start a timer for display refresh
  
  readMem(); // get stored values

  initWiFi();
     // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

// Setup the display
    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    int16_t tbx, tby; uint16_t tbw, tbh;
    uint16_t x = display.width() ;
    uint16_t y = display.height() ; // y is base line!
    // show what happens, if we use the bounding box for partial window
    display.init(); // enable diagnostic output on Serial
    updateStatic();  // This refreshes the screen 
    Serial.println("setup done");
}
void printLocalTime(){
  struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
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
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
}


void initWiFi() {
  int failcnt = 0; 
  WiFi.mode(WIFI_STA);
    Serial.print("Connecting to WiFi:"); 
    Serial.print(ssid); 
    Serial.print(" "); Serial.println(password);
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.print('.');
    delay(500);
    if (failcnt++ > 25){
      return;
      break;
    };
  };
  Serial.printf("Connected to SSID=%s IP address=", ssid);
  Serial.println(WiFi.localIP());
  onlineReady = true; 
  wifiStatus = "b"; /// see if the icon changes

}
void getVoltage(){
int vValue;
  // Reading potentiometer value
  vValue = analogRead(vPin);
  volts=vValue * 3.3/4096;
#ifdef DEBUG
//   Serial.printf("reading is %d and voltage is %5.3f \n",vValue, volts);
#endif
  delay(500);
}

void readMem(){
 if (prefs.begin("name",true)){
    firstName = prefs.getString("FirstName");
    lastName = prefs.getString("Lastname");
//    empNo = prefs.getUInt("Empno");
    empNo = prefs.getString("Empno");
        prefs.end(); // close for name
  }
  else{
    Serial.println("Error retrieving the name memory storage.");
  };
  if (prefs.begin("credentials",true)){
    ssid = prefs.getString("SSID");
    password = prefs.getString("wifiPassword");
    prefs.end(); // close for credentials
  }
  else{
    Serial.println("Error retreiving the credentials memory storage.");
  };
#ifdef DEBUG
//  Serial.printf("First Name:%s Last Name:%s\n",firstName,lastName);
//  Serial.printf("SSID:%s Password:%s\n",SSID,wifiPassword);
//  Serial.printf("Employee:%u\n",empNo);
#endif
};
void updatePWM(){
    throbberled.Update();
    resetled.Update();
    powerled.Update();
    SAO1led.Update();
    SAO2led.Update();
};
void showPartialUpdate(String inputString){
  // some useful background
  // use asymmetric values for test
  uint16_t box_x = 150; // was 130 
  uint16_t box_y = YSTATUS-10; // was 120
  uint16_t box_w = 100;
  uint16_t box_h = 15;
/*
  uint16_t box_x = 150;
  uint16_t box_y = 120;
  uint16_t box_w = 100;
  uint16_t box_h = 10;
*/
//  uint16_t cursor_y = box_y + box_h - 6;
//  if (display.epd2.WIDTH < 104) {Serial.println("width check passed"); cursor_y = box_y + 6;};
//  uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
  display.setFont(&TomThumb);
  //if (display.epd2.WIDTH < 104) display.setFont(0);
  display.setTextColor(GxEPD_BLACK);
  // show where the update box is
    display.setRotation(1);
    display.setPartialWindow(box_x, box_y, box_w, box_h);
    display.firstPage();
    do
    {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
   
      display.setCursor(XSTATUS,YSTATUS);
      display.setFont(&TomThumb);
      display.print(DISVERSION); 
      display.setRotation(1);
      display.setCursor(XSTATUS-65,YSTATUS);
      getVoltage();
      display.print(String(volts) + " volts");
      display.setCursor(XSTATUS-30,YSTATUS);
      display.setFont(&WIFI12pt7b);
      display.print(wifiStatus);
      display.setFont(&TomThumb);
      display.setCursor(box_x,YSTATUS);
      display.print(inputString);
      //display.fillScreen(GxEPD_BLACK);
    }
    while (display.nextPage());

    delay(1000);
    display.firstPage();
}
void updateStatic(){
    display.setRotation(1);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setPartialWindow(0, 0, display.width(), display.height());
//    display.update();
//    Serial.printf("Width=%u Height=%u\n",GxEPD_WIDTH,GxEPD_HEIGHT);
    
    //    display.setFont(&FreeMonoBold18pt7b);

    display.setFont(&FreeMono18pt7b);
    display.drawBitmap(0,15,epd_bitmap_Tandem_Chevron_inverted,34,34,GxEPD_BLACK);
// first name size depends on its length
//     display.update();
    display.setCursor(45,66);
    if(firstName.length() < 6){
      display.setCursor(45,66);
      display.setFont(&data_latin42pt7b);
    }else
    if (firstName.length() < 8){
      display.setCursor(45,50);
      display.setFont(&data_latin30pt7b);
    }else
    if (firstName.length() < 9){
      display.setCursor(45,40);
      display.setFont(&data_latin24pt7b);
    }else
    if (firstName.length() < 11){
      display.setCursor(45,40);
      display.setFont(&data_latin20pt7b);  
    }else
    if (firstName.length() < 12){
      display.setCursor(45,40);
      display.setFont(&data_latin18pt7b);
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
    };
    display.println(lastName);
  //  display.update();

    display.setFont(&data_latin10pt7b);
    display.setCursor(45,120);
    display.print("#") ; display.println(empNo);
   
//    partialScreenBuffer = "Test note " ;
    display.nextPage(); // display the page 
    int pg = display.pages();
#ifdef DEBUG
    Serial.printf("Number of pages=%u\n",pg);
#endif
    delay (1000); 
    showPartialUpdate(partialScreenBuffer);
};
void ledSetup(int which){
  switch (which) {
    case B000:   // cylon inverse
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
    case B001:   // countup (cumulative on)
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
    case B010:   // slide over could ust use <<1 instead
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
 case B011:   // slide on down (off)
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
 case B100:   // slide around
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
 case B101:   // slide on down (on)
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
 case B110:   // count up (off)
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
 case B111:   // cylon on
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
#ifdef DEBUG
  print_bytes(ledArray[ledCounter]);
  Serial.println(ledCounter);
#endif
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
    return true;
  };

}; // end of switchCheck

  void processSwitches(){
  int which;
  
  if (result == SLEEPSWITCHEDON){
    timeToSleep = true;
    ledResult = 0; //hard code to shut off the leds
    writeLEDs();
    };
  if (result == UNIVERSALNUMBER) { 
    which = random(0,7);
    ledSetup(which);
    specialSwitchSet = true;
  };
  if (result == CYLON) { 
    which = 0B111;
    ledSetup(which);
    ledRefreshDelay = 35;  // slower for cylon
    specialSwitchSet = true;
  };
  if (result == LOADREGISTERS) { 
    which = 0B110;
    ledSetup(which);
    ledRefreshDelay = 0750;  // slower for load registers
    noReverse = true;
    specialSwitchSet = true;
  };
  ledResult = reverseBits(result); //This sets up the proper order for the physical switches 
  };


  void loop()
{

    updatePWM();

    if (switchCheck(result)){
      processSwitches();
    };
    writeLEDs(); // this procedure will look to see if it is array or standard value
    if (refreshTimer.available()) {
    refreshTimer.stop();

    String partialStringBuffer = "";

    showPartialUpdate(partialStringBuffer);
    refreshTimer.start(REFRESHTIMERVAL);// Every minute
  }
  if (timeToSleep){
    ledResult = 0;
    writeLEDs(); // this procedure will look to see if it is array or standard value
    showPartialUpdate("Sleep...z.z...");
    timeToSleep = false; // not needed since wake forces a setup to execute. Just in case. 
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
//    display.powerDown();
    esp_deep_sleep_start();

  };
};
