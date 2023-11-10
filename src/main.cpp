#include "AiEsp32RotaryEncoder.h"
#include <ESP32Encoder.h>
#include "Arduino.h"


#if defined(ESP8266)
#define ROTARY_ENCODER_A_PIN D6
#define ROTARY_ENCODER_B_PIN D5
#define ROTARY_ENCODER_BUTTON_PIN D7
#else
#define CLK 2 // CLK ENCODER
#define DT 15 // DT ENCODER
#define ROTARY_ENCODER_BUTTON_PIN 4
#endif
#define ROTARY_ENCODER_VCC_PIN -1 /* 27 put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */

//depending on your encoder - try 1,2 or 4 to get expected behaviour
#define ROTARY_ENCODER_STEPS 1
// #define ROTARY_ENCODER_STEPS 2
// #define ROTARY_ENCODER_STEPS 4

//instead of changing here, rather change numbers above
// AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(NULL, NULL, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
ESP32Encoder encoder;

//paramaters for button
unsigned long shortPressAfterMiliseconds = 50;   //how long short press shoud be. Do not set too low to avoid bouncing (false press events).
unsigned long longPressAfterMiliseconds = 1000;  //how long čong press shoud be.

void on_button_short_click() {
  Serial.print("button SHORT press ");
  Serial.print(millis());
  Serial.println(" milliseconds after restart");
}

void on_button_long_click() {
  Serial.print("button LONG press ");
  Serial.print(millis());
  Serial.println(" milliseconds after restart");
}

void handle_rotary_button() {
  static unsigned long lastTimeButtonDown = 0;
  static bool wasButtonDown = false;

  bool isEncoderButtonDown = rotaryEncoder.isEncoderButtonDown();
  //isEncoderButtonDown = !isEncoderButtonDown; //uncomment this line if your button is reversed

  if (isEncoderButtonDown) {
    Serial.print("+");  //REMOVE THIS LINE IF YOU DONT WANT TO SEE
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
    if (millis() - lastTimeButtonDown >= longPressAfterMiliseconds) {
      on_button_long_click();
    } else if (millis() - lastTimeButtonDown >= shortPressAfterMiliseconds) {
      on_button_short_click();
    }
  }
  wasButtonDown = false;
}


void rotary_loop() {
  //dont print anything unless value changed
  if (rotaryEncoder.encoderChanged()) {
    Serial.print("Value: ");
    Serial.println(rotaryEncoder.readEncoder());
  }
  handle_rotary_button();
}

void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}

void setup() {
  Serial.begin(9600);

  encoder.attachHalfQuad(DT, CLK);
  encoder.setCount(0);

  //we must initialize rotary encoder
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  rotaryEncoder.disableAcceleration(); 
}

void loop() {
  //in loop call your custom function which will process rotary encoder values
  rotary_loop();
  long newPosition = encoder.getCount();
  Serial.println(newPosition);
  delay(50);  //or do whatever you need to do...
}
