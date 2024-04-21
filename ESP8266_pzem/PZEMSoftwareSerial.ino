#define BLYNK_TEMPLATE_ID "TMPL6gFicPlha"
#define BLYNK_TEMPLATE_NAME "CONG TO DIEN 3"
#define BLYNK_AUTH_TOKEN "NXnS7ZW3mVTTGY5pwOINpCOrUFdENHJa"

#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <SPI.h>
#include <Wire.h>
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#define SW_PIN 0
unsigned long TimeCount=0;
unsigned long timeReadPzem = millis();
#define DIENAP   V0
#define DONGDIEN V1
#define CONGSUAT V2
#define DIENNANG V3
#define CONTROL  V4
#define RELAY    13
char blynk_token[] = BLYNK_AUTH_TOKEN;
BlynkTimer timer;
byte RLStatus=0;
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);
#if defined(ESP32)
    #error "Software Serial is not supported on the ESP32"
#endif

/* 
 * Pin 12 Rx (Connects to the Tx pin on the PZEM)
 * Pin 13 Tx (Connects to the Rx pin on the PZEM)
*/
#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 12
#define PZEM_TX_PIN 14
#endif
SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem(pzemSWSerial);

void controlON()
{
  digitalWrite(RELAY,LOW);
  RLStatus=1;
}
void controlOFF()
{
  digitalWrite(RELAY,HIGH);
  RLStatus=1; 
}
void setup() {
    Serial.begin(115200);
    pinMode(RELAY,OUTPUT);
    controlOFF();
    connect();
    lcd.init();
    lcd.backlight();
    lcd.setCursor(3,0);
    lcd.print("WELCOME!");
    lcd.setCursor(1,1);
    lcd.print("INITIALIZATION");
    lcd.init(); 
}

void loop() {
    Blynk.run();
         
    Serial.print("Custom Address:");
    Serial.println(pzem.readAddress(), HEX);

    // Doc gia tri cam bien
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();

    // kiem tra cam bien
    if(isnan(voltage)){
        Serial.println("Error reading voltage");
    } else if (isnan(current)) {
        Serial.println("Error reading current");
    } else if (isnan(power)) {
        Serial.println("Error reading power");
    } else if (isnan(energy)) {
        Serial.println("Error reading energy");
    } else if (isnan(frequency)) {
        Serial.println("Error reading frequency");
    } else if (isnan(pf)) {
        Serial.println("Error reading power factor");
    } else {

        // in ra serial monitor
        Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
        Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
        Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
        Serial.print("Energy: ");       Serial.print(energy,3);     Serial.println("kWh");
        Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");
        Serial.print("PF: ");           Serial.println(pf);

        Blynk.virtualWrite(DIENAP,voltage);
        Blynk.virtualWrite(DONGDIEN,current);
        Blynk.virtualWrite(CONGSUAT,power);
        Blynk.virtualWrite(DIENNANG,energy);
        
    }
    lcd.clear();
    
    lcd.setCursor(0,0);
    lcd.print("U:");
    lcd.print(voltage,1);

    lcd.setCursor(9,0);
    lcd.print("I:");
    lcd.print(current,2);

    lcd.setCursor(0,1);
    lcd.print("P:");
    lcd.print(power,2);

    lcd.setCursor(9,1);
    lcd.print("E:");
    lcd.print(energy,2);
    timeReadPzem = millis();
    Serial.println();
    delay(2000);
}
 void connect()
 {
  WiFiManager wifiManager;// Khởi tạo đối tượng cho WiFiManager
  Serial.println("Delete old wifi? Press Flash button within 3 second");
  for(int i=3;i>0;i--)
  {
    Serial.print(String(i)+" ");
    delay(1000);
      }
  if(digitalRead(SW_PIN)==LOW)// press button
  {
   Serial.println();
   Serial.println("Reset wifi config!");
   wifiManager.resetSettings();   
  }
    WiFiManagerParameter custom_blynk_token("Blynk", "blynk token", blynk_token, 34);
    wifiManager.addParameter(&custom_blynk_token);
    wifiManager.autoConnect("ESP8266_PZEM004T","12345678");// tai khoan, mat khau wifi esp8266 phat ra
    //wifiManager.autoConnect();// use this to display host as ESP name + CHIPID
    // if can go next mean already connected wifi
    Serial.println("YOU ARE CONNECTED TO WIFI");
    Serial.println(custom_blynk_token.getValue());
    Blynk.config(custom_blynk_token.getValue());
    if (WiFi.status() == WL_CONNECTED) 
  {
   Serial.println("WIFI CONNECTED SUCCESSFULLY! NOW TRY TO CONNECT BLYNK..."); 
   delay(1000); 
   Blynk.connect(3333); // try to connect to Blynk with time out 10 second
   if(Blynk.connected()) 
   {
    Serial.println("BLYNK CONNECTED! System ready"); 

    delay(1000);
   }
   else 
   {
    Serial.println(" BLYNK Not connect. warning!");
    delay(1000);
   }
  }
  else 
  {
    Serial.println("WIFI Not connect. warning!");
    delay(1000);
  }
}

BLYNK_CONNECTED() {
  Blynk.syncAll();
}

BLYNK_WRITE(V4)
{
  int pinValue = param.asInt();
  if(pinValue==1) 
  {
    controlON();
  }
  else
  {
    controlOFF();
  }
  TimeCount=millis();
}
