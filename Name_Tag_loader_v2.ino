/*
  This is just for loading the permanent storage into the LilyGo ESP. It is not the primary display. 
  See the badge software ("Badge....ino") for the software the uses the persistant values that this tool places into the device. 
*/
#define SERIALTIMEOUT 1000
#include <Preferences.h>


String firstName;
String lastName;
String wifiPassword;
String SSID;
String wifiPassword2;
String SSID2;
String input;
String empNo;

// Set up to use flash memory preference library
Preferences prefs;

void setup(void)
{
    Serial.begin(115200);
    Serial.println();
//    Serial.println("setup");
    Serial.println("*** Tool for entering persistent badge data. This tool is not the full badge.");
    Serial.println("\n Current Values:");
    readMem();
}

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

bool getName(){
  int len = 0;
  Serial.setTimeout(SERIALTIMEOUT);
  Serial.flush();
  Serial.print("\nEnter first Name:");
  len = getResponse();
  //strcpy((char *)firstName,input);
  if (len < 1) return false;
  firstName = input;
  firstName.trim(); 
  Serial.println(firstName);  
  
  //delay(1000);
  Serial.print("Enter last name:"); 
  len = getResponse();
  if (len < 1) return false;
  lastName=input;
  lastName.trim();
  Serial.println(lastName);
  Serial.println();
  //delay(1000);

  Serial.print("Employee num (any value):"); 
  len = getResponse();
  if (len < 1) return false;
  //empNo = input.toInt();
  empNo = input;
  if (len > 8)
    empNo.substring(0,8);
  Serial.println(empNo);
 
  Serial.print("Enter Primary SSID:");
  len = getResponse();
  if (len < 1) return false;
  SSID=input;
  SSID.trim();
  Serial.println(SSID);
  delay (1000);
  Serial.print("Enter Primary WiFi Password:");
  len = getResponse();
  if (len < 1) return false;
  wifiPassword=input;
 // wifiPassword=String((char *)input);
  wifiPassword.trim();
  Serial.println(wifiPassword);
  Serial.println();

   Serial.print("Enter backup SSID:");
  len = getResponse();
  if (len < 1) return false;
  SSID2=input;
  SSID2.trim();
  Serial.println(SSID2);
  delay (1000);
  Serial.print("Enter backup WiFi Password:");
  len = getResponse();
  if (len < 1) return false;
  wifiPassword2=input;
 // wifiPassword=String((char *)input);
  wifiPassword2.trim();
  Serial.println(wifiPassword2);
  Serial.println();

  if (valid())
    return true;
  else
    return false;
};

bool valid(){
  Serial.println("Current values:");
  Serial.printf("First name=%s\nLast name=%s\nEmployee num=%s\n",firstName,lastName,empNo);
  Serial.printf("SSID=%s\nPassword=%s\n",SSID,wifiPassword);
  Serial.printf("SSID2=%s\nPassword2=%s\n",SSID2,wifiPassword2);
  Serial.print("Should I place this into memory? (y/n)");
 // Serial.flush();
  getResponse();
  String yn = input;
  yn.trim(); 
  Serial.println("");
  Serial.println(yn);
  input = yn.substring(0,1);
  if (yn == "y" || yn == "Y"){
    return true;
  }
  else{
    Serial.println("Rejected. Try again.");
   return false;  
  };
}; // end validate

bool user_input(){
  Serial.flush();
    Serial.print(">");
  if (Serial.available()){
  getResponse();
  };
  String yn = input;
  yn.trim(); 
  //Serial.println("");
  Serial.println(yn);
  input = yn.substring(0,1);
  if (yn == "y" || yn == "Y"){
    return true;
  }
  else{
//    Serial.println("Rejected. Try again.");
   return false;  
  };
}; // end User_input
void updateMem(){
  if (prefs.begin("name")){
    Serial.print("Updating name...");
    prefs.clear();
    prefs.putString("FirstName",firstName);
    prefs.putString("Lastname",lastName);
//    prefs.putUInt("Empno",empNo);
    prefs.putString("Empno",empNo);
    prefs.putString("SSID",SSID);
    prefs.end(); // close for name
    Serial.print("Reading name...");
    prefs.getString("FirstName",firstName);
    prefs.getString("Lastname",lastName);
//    prefs.getUInt("Empno",empNo);
    prefs.getString("Empno",empNo);
    Serial.printf("FN=%s LN=%s EMP=%s SSID=%s PW=%s SSID2=%s PW2=%s\n",firstName,lastName,empNo,SSID,wifiPassword,SSID2,wifiPassword2);
  }
  else{
    Serial.println("Error storing the name memory storage.");
  };
  prefs.end(); // close for name

  if (prefs.begin("credentials")){
    Serial.print("Updating credentials...");
    prefs.putString("SSID",SSID);
    prefs.putString("wifiPassword",wifiPassword);
    prefs.putString("SSID2",SSID2);
    prefs.putString("wifiPassword2",String(wifiPassword2));
    prefs.end(); // close for credentials
    Serial.print("Reading credentials...");
    prefs.getString("SSID",String(SSID));
    prefs.getString("wifiPassword",String(wifiPassword));
    Serial.printf("SSID=%s PW=%s\n",SSID,wifiPassword);
    prefs.getString("SSID2",String(SSID2));
    prefs.getString("wifiPassword2",String(wifiPassword2));
    Serial.printf("SSID2=%s PW2=%s\n",SSID2,wifiPassword2);
  }
  else{
    Serial.println("Error storing the credentials memory storage.");
  };
  prefs.end();
#ifdef DEBUG
  Serial.printf("First Name:%s Last Name:%s\n",firstName,lastName);
  Serial.printf("SSID:%s Password:%s\n",SSID,wifiPassword);
  Serial.printf("Employee:%u\s",empNo);
#endif
};

void readMem(){
 if (prefs.begin("name",true)){
    Serial.print("Reading name...");
    firstName = prefs.getString("FirstName");
    lastName = prefs.getString("Lastname");
    empNo = prefs.getString("Empno");
    prefs.end(); // close for name
    Serial.printf("FN=%s LN=%s Emp=%s\n",firstName,lastName,empNo);
  }
  else{
    Serial.println("Error retrieving the name memory storage.");
  };
  if (prefs.begin("credentials",true)){
    Serial.print("Reading credentials...");
    SSID = prefs.getString("SSID");
    wifiPassword = prefs.getString("wifiPassword");
    SSID2 = prefs.getString("SSID2");
    wifiPassword2 = prefs.getString("wifiPassword2");

    prefs.end(); // close for credentials
    Serial.printf("SSID=%s PW=%s SSID2=%s PW2=%s\n",SSID,wifiPassword,SSID2,wifiPassword2);
  }
  else{
    Serial.println("Error retreiving the credentials memory storage.");
  };
#ifdef DEBUG
  Serial.printf("First Name:%s Last Name:%s\n",firstName,lastName);
  Serial.printf("SSID:%s Password:%s\n",SSID,wifiPassword);
  Serial.printf("Employee:%s\n",empNo);
#endif
};



void loop()
{
//    readMem();
    delay (500);  
    if (getName()){
      updateMem();
    };
    readMem();
    delay (1000);
};
