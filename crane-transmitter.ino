
/*
Author: Bello  shehu
Title: Crane remote control transmitter
Description: Transmitter circuit using NRF for remote control of mini-crane. 
Date: 30/07/2025
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ezButton.h>

// --- MAIN JOYSTICK PINS---
#define SW_PIN 2
#define VRX_PIN 1
#define VRY_PIN 0

// --- AUX JOYSTICK PINS---
#define AUX_SW_PIN A2
#define AUX_VRX_PIN A0
#define AUX_VRY_PIN A1

// --- BUTTON PINS ---
#define BTN_LEFT_PIN 3 // sw1
#define BTN_FORWARD_PIN 4 // sw2
#define BTN_RIGHT_PIN 5 // sw3
#define BTN_BACKWARD_PIN 6 // sw4
#define BTN_HOOK_UP_PIN 7 // sw5: button for controlling hook downward
#define BTN_HOOK_DOWN_PIN 8 // sw6: button for controlling the hook upward
#define BTN_ROTATE_LEFT_PIN 19 // sw5: button for controlling rotation left
#define BTN_ROTATE_RIGHT_PIN 17 // sw6: button for controlling the rotation right

// --- NRF MODULE ---
#define NRF_MISO_PIN 12
#define NRF_MOSI_PIN 11
#define NRF_SCK_PIN 13
#define NRF_CSN_PIN 10
#define NRF_CE_PIN 9

// --- LED PINS
#define LED_SIGNAL 18
// #define LED_POWER 19

// --- VOLTAGE SEBNSOR
#define VOLT_SENSOR_PIN A3
// Floats for ADC voltage & Input voltage
float adc_voltage = 0.0;
// Floats for resistor values in divider (in ohms)
float R1 = 30000.0;
float R2 = 7500.0; 
// Float for Reference Voltage
float ref_voltage = 5.0;
// Integer for ADC value
int adc_value = 0;

// JOYSTICK THRESHOLDS
/*

          0
          |
          |
  0 ---- 512 ---- 1024
          |
          |
         1024
         
*/

#define LEFT_THRESHOLD 450 // anything less 450 is condsidered left
#define RIGHT_THRESHOLD 600 // anything 600 or higher is considered right
#define UP_THRESHOLD 400 // anything 400 or more
#define DOWN_THRESHOLD 330 // anything 300 or less is down
const int xAxisNeutral = 310;
const int yAxisNeutral = 460;
// BUTTONS
ezButton btnLeft(BTN_LEFT_PIN);
ezButton btnForward(BTN_FORWARD_PIN);
ezButton btnRight(BTN_RIGHT_PIN);
ezButton btnBackward(BTN_BACKWARD_PIN);
ezButton btnHookUp(BTN_HOOK_UP_PIN);
ezButton btnHookDown(BTN_HOOK_DOWN_PIN);
ezButton btnRotateLeft(BTN_ROTATE_LEFT_PIN);
ezButton btnRotateRight(BTN_ROTATE_RIGHT_PIN);

RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);
const byte address[6] = "00011";

unsigned int val[2] ;
unsigned long lastTimeVoltageReading = 0;
byte ledState = LOW;

struct JoystickValues {
  int x = 0;
  int y = 0;
};

int boomYValue, boomXValue = 0;
byte data = 0;  // Create a variable with the above structure
void toggleSignalLED(byte state = LOW);

void setup() {
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  //Serial.begin(9600);
  pinMode(LED_SIGNAL, OUTPUT);
  //pinMode(LED_POWER, OUTPUT);

  digitalWrite(LED_SIGNAL, HIGH);
  digitalWrite(LED_SIGNAL, HIGH);
  delay(1000);
  digitalWrite(LED_SIGNAL, LOW);
  digitalWrite(LED_SIGNAL, LOW);
  delay(1000);
}

void loop() {
  JoystickValues val = getAverage();
  boomXValue = analogRead(AUX_VRX_PIN);
  boomYValue = analogRead(AUX_VRY_PIN);
;
  // Serial.println("yValue:"+ String(boomYValue));
  // Serial.println("xValue:"+ String(boomXValue));
  
  if(digitalRead(BTN_LEFT_PIN)== LOW){
    data = 4;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(data));
    //Serial.println("drive left");
  }
  else if(digitalRead(BTN_RIGHT_PIN)== LOW){
    data = 2;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(data));
    //Serial.println("drive right");
  }
  else if(digitalRead(BTN_FORWARD_PIN)== LOW){
    data = 1;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(data));
    //Serial.println("drive forward");
  }
  else if(digitalRead(BTN_BACKWARD_PIN)== LOW){
    data = 3;
    toggleSignalLED(HIGH);
     radio.write(&data, sizeof(data));
    //Serial.println("drive back");

  }
  else if(digitalRead(BTN_HOOK_UP_PIN)== LOW){
    data = 7;
    radio.write(&data, sizeof(data));
   //Serial.println("hook up");
  }
  else if(digitalRead(BTN_HOOK_DOWN_PIN)== LOW){
    data = 8;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(data));
    //Serial.println("hook down");
  }

  // check left/right joystick  presss to control boom 
  else if ((boomYValue > 600 && boomXValue >= 410)) {
   // boom up
    data = 9;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(data));
  } else if ((boomYValue < 360 && boomXValue <= 240)) {
   // boom down
    data = 10;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(data));
  }

  // check up/donw press to rotate left right
  else if (digitalRead(BTN_ROTATE_RIGHT_PIN)== LOW) {
   //Serial.println("boom left");
    data = 11;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(data));
    //Serial.println("boom left");
  } else if (digitalRead(BTN_ROTATE_LEFT_PIN)== LOW ) {
   // Serial.println("boom right");
    data = 12;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(data));
   // Serial.println("boom right");
  }else{
    data = 255;
    radio.write(&data, sizeof(data));
  }
  // Send the whole data from the structure to the receiver
  // radio.write(&data, sizeof(Data_package));
  // radio.write(&data, sizeof(data));
  toggleSignalLED(LOW);
  scanBatteryVoltage();
  delay(10);
}

void toggleSignalLED(byte state=LOW){
  digitalWrite(LED_SIGNAL, state);
}

// void togglePowerLED( byte state=LOW){
//   digitalWrite(LED_POWER, state);
// }

float getVoltage(){
  float adc_value = analogRead(VOLT_SENSOR_PIN);
  // Determine voltage at ADC input
  adc_voltage  = (adc_value * ref_voltage) / 1024.0;
  // Calculate voltage at divider input
  float voltage = adc_voltage*(R1+R2)/R2;
  return voltage;
}

void scanBatteryVoltage(){
  if(millis() - lastTimeVoltageReading > 2000){
    float volt = getVoltage();
    if(volt < 10){
      // turn blink RED LED when less 10v
      if(ledState){
        //togglePowerLED(LOW);
      }else{
        //togglePowerLED(HIGH);
      }
      ledState = !ledState;
    }else {
      // turn LED 
      //togglePowerLED(HIGH);
    }
    lastTimeVoltageReading = millis();
   //Serial.println(volt);
  }
}

JoystickValues getAverage(){
  int sumX = 0;
  int sumY = 0;
  JoystickValues xyValues;
  xyValues.x = 0;
  xyValues.y = 0;
  for(int i = 0; i < 5; i++){
     sumX = sumX + analogRead(AUX_VRX_PIN);
     sumY = sumY + analogRead(AUX_VRY_PIN);
     delay(10);
  }
  xyValues.x = sumX / 10;
  xyValues.y = sumY / 10;
  return xyValues;
}