#include "Arduino.h"
#include "AiEsp32RotaryEncoder.h"
#include <ESP32Encoder.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "vivo Y21";
const char* password = "00000000";
// const char* ssid = "123";
// const char* password = "kikikiki";
// const char* ssid = "Wokwi-GUEST";
// const char* password = "";
// const char* scriptUrl = "https://script.google.com/macros/s/AKfycbxFQUYr70_3V211rjUAKNWNywPWm2TxhQTi7EQtfW640JutbGbK-SZjF2wSLtmsnwTh/exec";
const char* scriptUrl = "https://script.google.com/macros/s/AKfycbyC7ZqDqIWv-hdsJnuiwrIMsLN8pkMEBYLvA8XH8gLyzMSOgnhTGgzJM7IaJ_6TX5_N/exec";


//define pin
#define RE_CLK_PIN 2      // CLK ENCODER
#define RE_DT_PIN 15    // DT ENCODER
#define RE_BUTTON_PIN 4   // BUTTON ENCODER
#define RE_VCC_PIN -1     /* 27 put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */
#define WATER_FLOW_PIN  5
#define V_BAT_PIN 32
#define PRESSURE_PIN  33
#define LED_WIFI_PIN 13
#define LED_DATA_PIN 27


//rotary encoder
#define RE_STEPS 1



//define class
AiEsp32RotaryEncoder  rotaryEncoder = AiEsp32RotaryEncoder(NULL, NULL, RE_BUTTON_PIN, RE_VCC_PIN, RE_STEPS);
ESP32Encoder          encoder;
LiquidCrystal_I2C     lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
HTTPClient http; 


//parameter untuk tombol
unsigned long shortPress = 50;   //milisecond
unsigned long longPresss = 1000;  //milisecond

//rotary

//variable display
bool  display_update = 0;
float display_batt = 0;
uint8_t display_step = 0;

//variable menu
uint8_t menu_step = 0;
bool menu_change = 0;
bool menu_last_change = 0;

//data
uint8_t data_nomer = 0;
uint8_t last_data_nomer = 0;
bool data_send_allow = 0;
float data_batery = 0;
uint16_t data_manual = 1000;
int16_t last_data_manual = 9999;


//waterflow
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
long currentMillis2 = 0;
long previousMillis2 = 0;
int interval2 = 1000;
int testtipping = 0;


boolean ledState = LOW;
float calibrationFactor = 9; //4.5
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
bool read_waterflow_allow = 0;
bool read_pressure_allow = 0;
float pressure;
float max_pressure = 0;
float bar_pressure = 0;


void on_button_short_click() {
  Serial.print("button SHORT press ");

}

void on_button_long_click() {
   Serial.print("button LONG press ");
}

void handle_rotary_button() {
  static unsigned long lastTimeButtonDown = 0;
  static bool wasButtonDown = false;

  bool isEncoderButtonDown = rotaryEncoder.isEncoderButtonDown();
  //isEncoderButtonDown = !isEncoderButtonDown; //uncomment this line if your button is reversed

  if (isEncoderButtonDown) {
    if (!wasButtonDown) {
      //start measuring
      lastTimeButtonDown = millis();
    }
    //else we wait since button is still down
    wasButtonDown = true;
    return;
  }

  //button is up
  if (wasButtonDown) {
    Serial.println("");  //REMOVE THIS LINE IF YOU DONT WANT TO SEE
    //click happened, lets see if it was short click, long click or just too short
    if (millis() - lastTimeButtonDown >= longPresss) {
      on_button_long_click();
    } else if (millis() - lastTimeButtonDown >= shortPress) {
      
      menu_step++;
      display_update = 1;
       encoder.clearCount();
      on_button_short_click();
    }
  }
  wasButtonDown = false;
}

void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}

void lcd_setup(){
  lcd.init();  // initialize the lcd 
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("AKURASI WATER METER");
  lcd.setCursor(3,1);
  lcd.print("DIBUAT OLEH :");
  lcd.setCursor(6,2);
  lcd.print("MUHLISIN");
  lcd.setCursor(3,3);
  lcd.print("(1912200002xx)");
  delay(3000);
  lcd.clear();
}

void lcd_wifi_connect(){
  lcd.setCursor(1,0);
  lcd.print("MENGHUBUNGKAN ALAT");
  lcd.setCursor(5,1);
  lcd.print("KE WI-FI :");
  lcd.setCursor(0,2);
  lcd.print("SSID : ");
  lcd.setCursor(7,2);
  lcd.print(ssid);
  lcd.setCursor(0,3);
  lcd.print("PASS : ");
  lcd.setCursor(7,3);
  lcd.print(password);
  delay(2000);
}

void lcd_connected(){
  lcd.clear();
  lcd.setCursor(3,1);
  lcd.print("ALAT TERHUBUNG");
  lcd.setCursor(6,2);
  lcd.print("KE WI-FI");
  delay(2500);
  lcd.clear();
}

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
  testtipping++;
}

void setup()
{
  Serial.begin(9600);      

  // LED
  pinMode(LED_DATA_PIN, OUTPUT);
  pinMode(LED_WIFI_PIN, OUTPUT);
  digitalWrite(LED_DATA_PIN, HIGH);
  digitalWrite(LED_WIFI_PIN, HIGH);






  lcd_setup();      // Print a message to the LCD.

  lcd_wifi_connect();
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); Serial.println("Connecting to WiFi..."); 
  }
  digitalWrite(LED_WIFI_PIN, LOW);
  lcd_connected();

  encoder.attachHalfQuad(RE_CLK_PIN, RE_DT_PIN);
  encoder.setCount(0);
  //we must initialize rotary encoder
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  rotaryEncoder.disableAcceleration(); 



  //water flow
  pinMode(WATER_FLOW_PIN, INPUT_PULLUP);
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  attachInterrupt(digitalPinToInterrupt(WATER_FLOW_PIN), pulseCounter, FALLING);
  
  display_update = 1;
  menu_step = 0;

  data_batery = (0.00092*analogRead(V_BAT_PIN))+0.8;
  
  // if(data_batery > 4.5)data_batery = 5;
  Serial.println(data_batery);
}

void display(){
  
  lcd.clear();
  //display fix baris 1
  lcd.setCursor(0,0);
  lcd.print("V : ");
  lcd.setCursor(4,0);
  lcd.print(data_batery);
  // lcd.print(display_batt);
  lcd.setCursor(8,0);
  lcd.print("V");

  //dinamic display
  if(display_step == 0){
    lcd.setCursor(0,1);
    lcd.print("ALAT SIAP DIGUNAKAN");
    lcd.setCursor(3,2);
    lcd.print("TEKAN TOMBOL 1X");
    lcd.setCursor(1,3);
    lcd.print("UNTUK MEMULAI TEST");
  }
  else if(display_step == 1){

    lcd.setCursor(6,1);
    lcd.print("STEP 1 :");
    lcd.setCursor(0,2);
    lcd.print("ATUR NOMER PELANGGAN");
    lcd.setCursor(6,3);
    lcd.print("-<");
    lcd.setCursor(12,3);
    lcd.print(">+");
    lcd.setCursor(8,3);
    lcd.print("0000");
    if(data_nomer < 10) lcd.setCursor(11,3);
    if(data_nomer >= 10) lcd.setCursor(10,3);
    if(data_nomer >= 100) lcd.setCursor(9,3);
    if(data_nomer >= 1000) lcd.setCursor(8,3);
    lcd.print(data_nomer);
  }
  else if(display_step == 2){
    lcd.setCursor(6,1);
    lcd.print("STEP 2 :");
    lcd.setCursor(0,2);
    lcd.print("TUTUP STOP KRAN DAN");
    lcd.setCursor(0,3);
    lcd.print("ISI ALAT DENGAN AIR");
  }
  else if(display_step == 3){
    lcd.setCursor(6,1);
    lcd.print("STEP 3 :");
    lcd.setCursor(2,2);
    lcd.print("PROSES PENGUKURAN");
  }
  else if(display_step == 4){
    lcd.setCursor(0,1);
    lcd.print("step 4 : hasil");
    lcd.setCursor(19,0);
    lcd.print(display_step);
    lcd.setCursor(0,2);
    lcd.print("Volume Air :");
    lcd.setCursor(12,2);
    lcd.print(totalMilliLitres);
    lcd.setCursor(17,2);
    lcd.print("mL");
    lcd.setCursor(0,3);
    lcd.print("Tekanan Air : ");
    lcd.setCursor(14,3);
    lcd.print(bar_pressure);
    

  }
  else if(display_step == 5){
    lcd.setCursor(6,1);
    lcd.print("STEP 5 :");
    lcd.setCursor(0,2);
    lcd.print("INPUT UKUR MANUAL");
    lcd.setCursor(6,3);
    lcd.print("-<");
    lcd.setCursor(12,3);
    lcd.print(">+");
    lcd.setCursor(8,3);
    lcd.print("0000");
    if(data_manual < 10) lcd.setCursor(11,3);
    if(data_manual >= 10) lcd.setCursor(10,3);
    if(data_manual >= 100) lcd.setCursor(9,3);
    if(data_manual >= 1000) lcd.setCursor(8,3);
    lcd.print(data_manual);

  }
  else if(display_step == 6){
    lcd.setCursor(0,1);
    lcd.print("step 6 : kirim data");
    lcd.setCursor(19,0);
    lcd.print(display_step);
  }


}

void loop()
{

  // Serial.println(analogRead(V_BAT_PIN)*3.3/4095);
 
  if(display_update){
    display();
    display_update = 0;
  }
  handle_rotary_button();
  // menu_change = encoder.getCount();
  // Serial.println(menu_step);

  if(read_pressure_allow){
    currentMillis2 = millis();
    if (currentMillis2 - previousMillis2 > interval2) {
      pressure = analogRead(PRESSURE_PIN);

      if(max_pressure < pressure) max_pressure = pressure;

      Serial.print("pressure ADC : ");
      Serial.println(pressure);
      Serial.print("Max pressure ADC : ");
      Serial.println(max_pressure);

      bar_pressure = (max_pressure - 220) / 670; //670 didapat dari rumus persamaan (k)

      previousMillis2 = currentMillis2;
    }
  }
  
  if(read_waterflow_allow){
    currentMillis = millis();
    if (currentMillis - previousMillis > interval) {



    data_batery = (0.00092*analogRead(V_BAT_PIN))+0.8;
    Serial.print("V Batt : ");
    Serial.println(data_batery);
 
    // Print the flow rate for this second in litres / minute
    Serial.print("pulseCount: ");
    Serial.print(int(pulseCount));  // Print the integer part of the variable
    pulse1Sec = pulseCount;
    pulseCount = 0;

    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;

    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.println("L");
    }
  }

  if(menu_step == 1){


    display_step = 1;
    if(data_nomer != encoder.getCount()){
      display_update = 1;
    }
    data_nomer = encoder.getCount();
    
  }
  else if(menu_step == 2){
    
    display_step = 2;
    read_pressure_allow = 1;
    
    
  }
  else if(menu_step == 3){
    read_pressure_allow = 0;
    read_waterflow_allow = 1;
    display_step = 3;
    data_send_allow =1;
  }
  else if(menu_step == 4){
    display_step = 4;
    read_waterflow_allow = 0;
  }
  else if(menu_step == 5){
    display_step = 5;

    if(last_data_manual != encoder.getCount()){
      display_update = 1;
      last_data_manual = encoder.getCount();
      data_manual = data_manual + last_data_manual;
      Serial.print("data_manual : ");
      Serial.print(data_manual);
      Serial.print("get count : ");
      Serial.print(encoder.getCount());

    }

    if(data_manual > 2000 || data_manual < 0){
      data_manual = 1000;
    }

  }
  else if(menu_step == 6){
    display_step = 6;

     if(display_update){
      display();
      display_update = 0;
    }


    //generate data JSON

    //koneksi ke wifi
    
    if(data_send_allow){
         //kirim data
      digitalWrite(LED_DATA_PIN, LOW);
      http.begin(scriptUrl); 
      http.addHeader("Content-Type", "application/json"); 
      // String json = "{\"nomer\": 1,\"pressure\": 2,\"water_flow\": 3, \"battery\": 4}"; // Ganti dengan data sensor sesuai kebutuhan. 

      String json = "{\"nomer\": " + String(data_nomer) + ",\"pressure\": " + String(bar_pressure) + ",\"water_flow\": " + String(totalMilliLitres) + ", \"battery\": " + String(data_batery) + ",\"water_flowP\": " + String(data_manual) + "}";
      // String json = "{\"nomer\": " + String(data_nomer) + ",\"pressure\": " + String(max_pressure) + ",\"water_flow\": " + "899" + ", \"battery\": " + String(data_batery) + ",\"water_flowP\": " + String(data_manual) + "}";

      int cnt;
      int httpResponseCode;

      do{
        httpResponseCode = http.POST(json); 
        Serial.println(json);
        cnt++;
        if(cnt > 5) break;
        delay(1000);
      }while(httpResponseCode < 0);
      
      // int httpResponseCode = http.POST(json); 
      Serial.println(json);

      

      // terima respon pengiriman
      if (httpResponseCode > 0) { 
        Serial.print("HTTP Response Code: "); 
        Serial.println(httpResponseCode); 
      } 
      else { 
        Serial.print("HTTP Error: "); 
        Serial.println(httpResponseCode); 
      } 
      http.end();
      digitalWrite(LED_DATA_PIN, HIGH);

      // delay(5000); // Kirim data setiap 5 detik (sesuaikan dengan kebutuhan Anda).
      data_send_allow = 0;
    }
 
  }
  else if(menu_step == 7){
    menu_step = 0;
    display_step = 0;
    flowRate = 0;
    totalMilliLitres = 0;
    max_pressure = 0;
    data_manual = 1000;
    last_data_manual = 9999;
    testtipping = 0;
  }
 
  // delay(50);
}