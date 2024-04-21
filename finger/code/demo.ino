#include <FS.h>
#include <time.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include "Adafruit_Fingerprint.h"
#include "SoftwareSerial.h"
#include "webserver.h"

#define ENROLL_MODE true
#define RUN_MODE    false
bool fingerMode = RUN_MODE;
int result = 0;
uint8_t idFinger = 1;

struct Finger_t {
  uint8_t id;
  char userName[20];
};
Finger_t fg[20];

/////////////////////////////////// FINGER /////////////////////////////
SoftwareSerial mySerial(12, 14);//tx rx //D5,D6
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
void setupFinger(){
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("FingerPrint Sensor!");
    Serial.println(F("Reading sensor parameters"));
    finger.getParameters();
    Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
    Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
    Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
    Serial.print(F("Security level: ")); Serial.println(finger.security_level);
    Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
    Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
    Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate); 
  } else {
    Serial.println("Not Detect FingerPrint Sensor");
  }
}

/////////////////////////////////// LCD ////////////////////////////////
LiquidCrystal_I2C lcd(0x27, 16, 2);
void lcd_Init(){
  Wire.begin(2, 0);
  lcd.init();
  lcd.backlight();            // Bật đèn nền
  lcd.home();                 //Đưa con trỏ về vị trí 0,0
  lcd.print("     Start    ");
  lcd.setCursor(0, 1);
  lcd.print("     System   ");
}

/////////////////////////////////// WIFI ///////////////////////////////
ESP8266WebServer server(80); //Server on port 80
void wifi_Init(){
  while (WiFi.softAP("ESP8266 WiFI", "12345678") == false){
     Serial.print(".");
     delay(300);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("Start WebServer IP: 192.168.4.1");
  
  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/readData", readData); //This page is called by java Script AJAX
  server.on("/postForm",handlePostForm);
 
  server.begin();                  //Start server
  Serial.println("HTTP server started");
}

void setup (){
  Serial.begin(115200);
  lcd_Init();
  setupFinger();
  wifi_Init();
}

void loop(void){
  if(fingerMode == ENROLL_MODE){
    lcd.home();
    lcd.print(" Finger Enroll");
    ENROLL(); //Resgis
  }else{
    lcd.home();
    lcd.print("  Finger start");
    int temp = getFingerprintID(); //get
    if(temp != 255)
      result = temp;
  }
  server.handleClient();          //Handle client requests
}

uint8_t getNameIndex(uint8_t id){
  for(int i = 0; i< 20; i++){
    if(fg[i].id == id){
      return i;
    }
  }
  return 255;
}

//---------------------------------------WEB--------------------------------------------------------------//
void handleRoot() {
   String s = MAIN_page; //Read HTML contents
   server.send(200, "text/html", s); //Send web page
}

void readData() {
 if(fingerMode == RUN_MODE && result != 0){
    String data;
    if(result == 254)
      data = "{\"Id\":\""+ String("Unknow") +"\", \"Status\":\""+ "False" +"\"}";
    else{
      int i = getNameIndex(result);
      if(i != 255){
        lcd.setCursor(0, 1);
        lcd.print("id: ");
        lcd.print(idFinger);
        lcd.print(", name: ");
        lcd.print(fg[idFinger-1].userName);
        Serial.print("id: ");Serial.print(i); Serial.print(", name: "); Serial.println(fg[i].userName);
        data = "{\"Id\":\""+ String(result)+ " - " + String(fg[i].userName) +"\", \"Status\":\""+ "True" +"\"}";
      }
    }
    server.send(200, "text/plane", data);
    result = 0;
 }
}

void mapId(String str){
    int len = str.length() + 1;
    char tmp[20];
    str.toCharArray(tmp, len);
    strcpy(fg[idFinger-1].userName, tmp);
    fg[idFinger-1].id = idFinger;
    Serial.print("id: "); Serial.print(idFinger - 1); Serial.print(", name: "); Serial.println(fg[idFinger-1].userName);
}

void handlePostForm(){
   String s = MAIN_page;
   String notice = server.arg("myText");
   Serial.println(notice);
   server.send(200,"text/html", s);
   fingerMode = ENROLL_MODE;
   mapId(notice);
}

//---------------------------------------ENROLL------------------------------------//
void ENROLL() {
  Serial.println("=================== Start enroll ==================");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  
  if (idFinger == 0) {// ID #0 not allowed, try again!
    return;
  }
  Serial.print("Enrolling ID #"); Serial.println(idFinger);
  while (!  getFingerprintEnroll() );
  Serial.println("=================== End enroll ==================");
}

//----RETURN NUM-------------------//
uint8_t getFingerprintEnroll() {
  if(fingerMode == RUN_MODE) return 1;
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(idFinger);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  lcd.setCursor(0, 1);
  lcd.print("Remove finger");

  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(idFinger);
  p = -1;
  Serial.println("Place same finger again");
  lcd.setCursor(0, 1);
  lcd.print("Place again");
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(idFinger);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
    lcd.setCursor(0, 1);
    lcd.print("FG matched");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    lcd.setCursor(0, 1);
    lcd.print("FG not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(idFinger);
  p = finger.storeModel(idFinger);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    lcd.setCursor(0, 1); //  id - name 
    lcd.print("  ");
    lcd.print(idFinger);
    lcd.print(" - ");
    lcd.print(fg[idFinger-1].userName);
    
    idFinger++;
    fingerMode = RUN_MODE;
    return p;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
}

//---------------------------------------ENROLL END--------------------------------//


//---------------------------------------FINGER -----------------------------------//
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return -1;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return -1;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return -1;
    default:
      Serial.println("Unknown error");
      return -1;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return -1;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return -1;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return -1;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return -1;
    default:
      Serial.println("Unknown error");
      return -1;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return -1;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return 254;
  } else {
    Serial.println("Unknown error");
    return -1;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

///////////////////////////////////////////////////////////////////////
