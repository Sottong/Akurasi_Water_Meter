#include "Arduino.h"
#include "AiEsp32RotaryEncoder.h"
#include <ESP32Encoder.h>
#include <LiquidCrystal_I2C.h>

//define pin
#define RE_CLK_PIN 2      // CLK ENCODER
#define RE_DT_PIN 15    // DT ENCODER
#define RE_BUTTON_PIN 4   // BUTTON ENCODER
#define RE_VCC_PIN -1     /* 27 put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */



//rotary encoder
#define RE_STEPS 1



//define class
AiEsp32RotaryEncoder  rotaryEncoder = AiEsp32RotaryEncoder(NULL, NULL, RE_BUTTON_PIN, RE_VCC_PIN, RE_STEPS);
ESP32Encoder          encoder;
LiquidCrystal_I2C     lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display



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
  lcd.print("Akurasi Water Meter");
  lcd.setCursor(3,1);
  lcd.print("Dibuat Oleh :");
  lcd.setCursor(6,2);
  lcd.print("Muhlisin");
  lcd.setCursor(3,3);
  lcd.print("(1912200002xx)");
  delay(3000);
  lcd.clear();
}

void setup()
{
  Serial.begin(9600);      
  lcd_setup();      // Print a message to the LCD.

  encoder.attachHalfQuad(RE_CLK_PIN, RE_DT_PIN);
  encoder.setCount(0);
  //we must initialize rotary encoder
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  rotaryEncoder.disableAcceleration(); 

  display_update = 1;
  menu_step = 0;
  
}


void display(){
  lcd.clear();

  //display fix baris 1
  lcd.setCursor(0,0);
  lcd.print("V:");
  lcd.setCursor(2,0);
  lcd.print("4.5");
  // lcd.print(display_batt);
  lcd.setCursor(5,0);
  lcd.print("V");

  //dinamic display
  if(display_step == 0){
    lcd.setCursor(0,1);
    lcd.print("TEKAN TOMBOL");
    lcd.setCursor(0,2);
    lcd.print("UNTUK");
    lcd.setCursor(0,3);
    lcd.print("MEMULAI TEST");
  }
  else if(display_step == 1){
    lcd.setCursor(0,1);
    lcd.print("step 1 : atur nomer");
    lcd.setCursor(19,0);
    lcd.print(display_step);
    lcd.setCursor(0,2);
    lcd.print(data_nomer);
  }
  else if(display_step == 2){
    lcd.setCursor(0,1);
    lcd.print("step 2 : perisapan");
    lcd.setCursor(19,0);
    lcd.print(display_step);
  }
  else if(display_step == 3){
    lcd.setCursor(0,1);
    lcd.print("step 3 : pengukuran");
    lcd.setCursor(19,0);
    lcd.print(display_step);
  }
  else if(display_step == 4){
    lcd.setCursor(0,1);
    lcd.print("step 4 : hasil");
    lcd.setCursor(19,0);
    lcd.print(display_step);
  }
  else if(display_step == 5){
    lcd.setCursor(0,1);
    lcd.print("step 5 : sending data");
    lcd.setCursor(19,0);
    lcd.print(display_step);
  }


}

void loop()
{

  if(display_update){
    display();
    display_update = 0;
  }
  handle_rotary_button();
  // menu_change = encoder.getCount();
  Serial.println(menu_step);
  
 

  if(menu_step == 1){


    display_step = 1;
    if(data_nomer != encoder.getCount()){
      display_update = 1;
    }
    data_nomer = encoder.getCount();
    
  }
  else if(menu_step == 2){
    display_step = 2;
  }
  else if(menu_step == 3){
    display_step = 3;
  }
  else if(menu_step == 4){
    display_step = 4;
  }
  else if(menu_step == 5){
    display_step = 5;
  }
  else if(menu_step == 6){
    menu_step = 0;
    display_step = 0;
  }
 
  
  

  


  delay(50);
}