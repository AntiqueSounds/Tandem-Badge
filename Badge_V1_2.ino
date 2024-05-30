#include <jled.h>
/*
First version of the badge - preferences must be loaded separately. 
M. Keith Moore
*/
#define ENABLE_GxEPD2_GFX 0
//#define DEBUG TRUE // Comment this out for non-debug
#define DISVERSION "V2"
#define XSTATUS 240  // beginning of the status @ this column
#define YSTATUS 120  // beginning of the status at this row
#define DATA_WIDTH 16
//#define UNIVERSALNUMBER 0B0000000000101010 // normally this should be 42 switches set to this turns on the blinkies
#define UNIVERSALNUMBER   0B1110000000000000 // this is a test value 7 (the are little endian left side- forced here to right side) 

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */ 
#define TIME_TO_SLEEP 10 /* Time ESP32 will go to sleep (in seconds) */
// 2 minutes
#define SERIALTIMEOUT 1000

#define LILYGO_T5_V213

#include <Preferences.h>
#include <boards.h>
#include <GxEPD.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_4C.h>
#include <GxEPD2_7C.h>
#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w  form DKE GROUP
#include <SD.h>
#include <FS.h>
#include <WiFi.h>

//#include GxEPD_BitmapExamples
// FreeFonts from Adafruit_GFX
// Fonts are located in the libraries/fonts directory under the Adafruit_GFX_Library folder in the folts folder. 

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
#include <Fonts/FreeMonoBold36pt7b.h>
#include <Fonts/FreeMonoBold42pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMono24pt7b.h>
#include <Fonts/FreeMono18pt7b.h>
#include <Fonts/FreeSansOblique24pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/TomThumb.h>

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
bool alternate = false; // set up for use of two different displays 

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
const int ledRefreshDelay = 100; // milliseconds of led time shown
uint16_t ledResult = 0;
boolean flipflop = true;  // used to reversing the LED displays 

const int vPin = 35;  // was 13 but the board shows pin 35 as battery test? 
const int throbberPin = 13;  // chevron pin 
const int SAO1Pin = 25;  // SAO pin
const int SAO2Pin = 26;  //  SAO pin
const int resetPin = 32; // Reset LED pin
const int powerPin = 33; // power LED pin

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

auto throbberled = JLed(throbberPin).Breathe(4500).DelayAfter(1000).Forever();
//auto throbberled = JLed(throbberPin).Candle(5 /* speed */, 150 /*jitter*/).Forever();
//auto resetled = JLed(resetPin).Breathe(3000).MinBrightness(10).DelayAfter(500).Repeat(50);
//auto powerled = JLed(powerPin).Breathe(2500).MinBrightness(10).DelayAfter(500).Repeat(50);
auto resetled = JLed(resetPin).Breathe(1000).MaxBrightness(30).MinBrightness(5).DelayAfter(500).Forever();
auto powerled = JLed(powerPin).Breathe(2000).MaxBrightness(30).MinBrightness(5).DelayAfter(500).Forever();
auto SAO1led = JLed(SAO1Pin).Candle(5 /* speed */, 100 /*jitter*/).Forever();
auto SAO2led = JLed(SAO2Pin).Candle(2 /* speed */, 200 /*jitter*/).Forever();

String firstName;
String lastName;
String wifiPassword;
String SSID;
String input;

int empNo;
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

bool setupSDCard(void)
{

    return SD.begin(SDCARD_CS);

}

void setup(void)
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");

#if defined(LILYGO_EPD_DISPLAY_102)
    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);
#endif /*LILYGO_EPD_DISPLAY_102*/
#if defined(LILYGO_T5_V102)
    pinMode(POWER_ENABLE, OUTPUT);
    digitalWrite(POWER_ENABLE, HIGH);
#endif /*LILYGO_T5_V102*/
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

  readMem(); // get stored values
    //display.init(115200); // default 10ms reset pulse, e.g. for bare panels with DESPI-C02
//    display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
    //display.init(115200, true, 10, false, SPI0, SPISettings(4000000, MSBFIRST, SPI_MODE0)); // extended init method with SPI channel and/or settings selection
    // first update should be full refresh
/*
    if (setupSDCard()) {
        uint8_t cardType = SD.cardType();

        if (cardType == CARD_NONE) {
            Serial.println("No SD card attached");
            return;
        }

        Serial.print("SD Card Type: ");
        if (cardType == CARD_MMC) {
            Serial.println("MMC");
        } else if (cardType == CARD_SD) {
            Serial.println("SDSC");
        } else if (cardType == CARD_SDHC) {
            Serial.println("SDHC");
        } else {
            Serial.println("UNKNOWN");
        }

        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.printf("SD Card Size: %lluMB\n", cardSize);
    } else {
        Serial.println("Card mount failed!");
    }
*/
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
    empNo = prefs.getUInt("Empno");
    prefs.end(); // close for name
  }
  else{
    Serial.println("Error retrieving the name memory storage.");
  };
  if (prefs.begin("credentials",true)){
    SSID = prefs.getString("SSID");
    wifiPassword = prefs.getString("wifiPassword");
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
void updateStatic2(){
    display.setRotation(1);
    display.fillScreen(GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.update();
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
    display.fillScreen(GxEPD_BLACK);
    display.update();

//    display.setFont(&FreeMonoBold18pt7b);
    display.setFont(&FreeMono18pt7b);
    display.fillScreen(GxEPD_BLACK);
   display.drawBitmap(5,15,epd_bitmap_Tandem_Chevron,34,34,GxEPD_WHITE);

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
  
    display.setCursor(XSTATUS,YSTATUS);
    display.setFont(&TomThumb);
    display.print(DISVERSION);
 
    display.setCursor(XSTATUS-40,YSTATUS);
    getVoltage();
    display.printf("%4.1f volts\n",volts) ;
#ifdef DEBUG
//    Serial.println(volts);
#endif
    display.println(volts) ;
    display.update();
    delay (1000); 
};
void updateStatic(){
    display.setRotation(1);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.update();
    Serial.printf("Width=%u Height=%u\n",GxEPD_WIDTH,GxEPD_HEIGHT);
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
    display.fillScreen(GxEPD_WHITE);
    display.update();

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
    display.setCursor(XSTATUS,YSTATUS);
    display.setFont(&TomThumb);
    display.print(DISVERSION); 
 
    display.setCursor(XSTATUS-40,YSTATUS);
    getVoltage();
    display.printf("%4.1f volts\n",volts) ;
#ifdef DEBUG
    Serial.println(volts);
#endif
    display.println(volts) ;
    display.update();
    delay (1000); 
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
    ledArray[0] = 32768;  //   B1000 0000 0000 0000;
    ledArray[1] = 1;      //   B0000 0000 0000 0001;
    ledArray[2] = 2;      //   B0000 0000 0000 0010;
    ledArray[3] = 4;      //   B0000 0000 0000 0100;
    ledArray[4] = 8;      //   B0000 0000 0000 1000; 
    ledArray[5] = 16;     //   B0000 0000 0001 0000;
    ledArray[6] = 32;     //   B0000 0000 0010 0000;
    ledArray[7] = 64;     //   B0000 0000 0100 0000;
    ledArray[8] = 128;    //   B0000 0000 1000 0000;
    ledArray[9] = 256;    //   B0000 0001 0000 0000;
    ledArray[10] = 512;   //   B0000 0010 0000 0000;
    ledArray[11] = 1024;  //   B0000 0100 0000 0000;
    ledArray[12] = 2048;  //   B0000 1000 0000 0000;
    ledArray[13] = 4096;  //   B0001 0000 0000 0000;
    ledArray[14] = 8192;  //   B0010 0000 0000 0000;
    ledArray[15] = 16384; //   B0100 0000 0000 0000;

    break;
 case B111:   // cylon on
    ledArray[0] = 32769;  //   B1000 0000 0000 0001;
    ledArray[1] = 49155;  //   B1100 0000 0000 0011;
    ledArray[2] = 57351;  //   B1110 0000 0000 0111;
    ledArray[3] = 61455;  //   B1111 0000 0000 1111;
    ledArray[4] = 63519;  //   B1111 1000 0001 1111;
    ledArray[5] = 64575;  //   B1111 1100 0011 1111;
    ledArray[6] = 65151;  //   B1111 1110 0111 1111;
    ledArray[7] = 65535;  //   B1111 1111 1111 1111;
    ledArray[8] = 65151;  //   B1111 1110 0111 1111;
    ledArray[9] = 65575;  //   B1111 1100 0011 1111;
    ledArray[10] = 63519; //   B1111 1000 0001 1111;
    ledArray[11] = 61455; //   B1111 0000 0000 1111;
    ledArray[12] = 57351; //   B1110 0000 0000 0111;
    ledArray[13] = 49155; //   B1100 0000 0000 0011;
    ledArray[14] = 32769; //   B1000 0000 0000 0001;
    ledArray[15] = 0;     //   B0000 0000 0000 0000;
    break;
  default:
#ifdef DEBUG
    Serial.println("Bad pattern switch selected");
#endif
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
    if (result != UNIVERSALNUMBER){
        shiftOut(dataPin,clockPin,MSBFIRST,(ledResult >> 8));   
        shiftOut(dataPin,clockPin,MSBFIRST,ledResult);
        digitalWrite(latchPin, HIGH);
        delay(50); 
    }
    else {
        if (ledCounter > 15 ){ // dont overflow
            ledCounter = 14;
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
#ifdef DEBUG 
// Print to serial monitor
//  Serial.print("Switch pin states:\r\n");
//   print_bytes(result);
#endif

if (result == lastResult) { 
    return false;
  }
  else{
    lastResult = result;
#ifdef DEBUG
    Serial.println("New switch value!");
#endif
    return true;
  };

}; // end of switchCheck

  void processSwitches(){
  if (result == UNIVERSALNUMBER) { 
    int which;
    which = random(0,7);
    ledSetup(which);
  };
  ledResult = reverseBits(result); //This sets up the proper order for the physical switches 
  };


  void loop()
{
    throbberled.Update();
    resetled.Update();
    powerled.Update();
    SAO1led.Update();
    SAO2led.Update();

    if (switchCheck(result)){
      processSwitches();
    };
    writeLEDs(); // this procedure will look to see if it is array or standard value
//    Serial.println("sleeping  ") ;
//    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
//    display.powerDown();
//   esp_deep_sleep_start();
};
