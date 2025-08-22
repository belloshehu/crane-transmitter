
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
#define BTN_HOOK_UP_PIN 7 // sw5
#define BTN_HOOK_DOWN_PIN 8 // sw6

// --- NRF MODULE ---
#define NRF_MISO_PIN 12
#define NRF_MOSI_PIN 11
#define NRF_SCK_PIN 13
#define NRF_CSN_PIN 10
#define NRF_CE_PIN 9

// --- LED PINS
#define LED_SIGNAL 18
#define LED_POWER 19

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

#define LEFT_THRESHOLD 400 // anything less 400 is condsidered left
#define RIGHT_THRESHOLD 540 // anything 500 or higher is considered right
#define UP_THRESHOLD 400 // anything 400 or more
#define DOWN_THRESHOLD 300 // anything 300 or less is down

// BUTTONS
ezButton btnLeft(BTN_LEFT_PIN);
ezButton btnForward(BTN_FORWARD_PIN);
ezButton btnRight(BTN_RIGHT_PIN);
ezButton btnBackward(BTN_BACKWARD_PIN);
ezButton btnHookUp(BTN_HOOK_UP_PIN);
ezButton btnHookDown(BTN_HOOK_DOWN_PIN);

RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);
const byte address[6] = "00011";

unsigned long lastTimeVoltageReading = 0;
byte ledState = LOW;

struct Data_package {
  byte left = 0;         // turns the vehicle to left
  byte right = 0;        // turns the vehicle to right
  byte forward = 1;      // moves the whole crane forward
  byte backward = 0;     // moves the whole crane backward
  byte up = 1;           // moves the crane up
  byte down = 0;         // moves the crane down
  byte rotateLeft = 0;   // rotates the crane right to left
  byte rotateRight = 1;  // rotates the crane left to right
  byte hookUp = 0;       // moves the hoot up
  byte hookDown = 0;     // moves the hook down
};

int boomYValue, boomXValue = 0;
Data_package data;  // Create a variable with the above structure
void toggleSignalLED(byte state = LOW);

void setup() {
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  // Serial.begin(9600);
  pinMode(LED_SIGNAL, OUTPUT);
  pinMode(LED_POWER, OUTPUT);

  digitalWrite(LED_SIGNAL, HIGH);
  digitalWrite(LED_SIGNAL, HIGH);
  delay(1000);
  digitalWrite(LED_SIGNAL, LOW);
  digitalWrite(LED_SIGNAL, LOW);
  delay(1000);
}

void loop() {
  // listen button presss
  btnLeft.loop();
  btnForward.loop();
  btnRight.loop();
  btnBackward.loop();
  btnHookUp.loop();
  btnHookDown.loop();
  
  if(btnLeft.isPressed()){
    data.up = 0;
    data.down = 0;
    data.rotateLeft = 0;
    data.rotateRight = 0;
    data.left = 1;
    data.right = 0;
    data.backward = 0;
    data.forward = 0;
    data.hookUp = 0;
    data.hookDown = 0;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(Data_package));

  }
  if(btnRight.isPressed()){
    data.up = 0;
    data.down = 0;
    data.rotateLeft = 0;
    data.rotateRight = 0;
    data.right = 1;
    data.left = 0;
    data.backward = 0;
    data.forward = 0;
    data.hookUp = 0;
    data.hookDown = 0;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(Data_package));

  }
  if(btnForward.isPressed()){
    data.up = 0;
    data.down = 0;
    data.rotateLeft = 0;
    data.rotateRight = 0;
    data.forward = 1;
    data.left = 0;
    data.right = 0;
    data.backward = 0;
    data.hookUp = 0;
    data.hookDown = 0;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(Data_package));

  }
  if(btnBackward.isPressed()){
    data.up = 0;
    data.down = 0;
    data.rotateLeft = 0;
    data.rotateRight = 0;
    data.backward = 1;
    data.left = 0;
    data.right = 0;
    data.forward = 0;
    data.hookUp = 0;
    data.hookDown = 0;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(Data_package));

  }
  if(btnHookUp.isPressed()){
    data.up = 0;
    data.down = 0;
    data.rotateLeft = 0;
    data.rotateRight = 0;
    data.hookUp = 1;
    data.hookDown = 0;
    data.backward = 0;
    data.left = 0;
    data.right = 0;
    data.forward = 0;
    radio.write(&data, sizeof(Data_package));
  }
  if(btnHookDown.isPressed()){
    data.up = 0;
    data.down = 0;
    data.rotateLeft = 0;
    data.rotateRight = 0;
    data.hookDown = 1;
    data.hookUp = 0;
    data.backward = 0;
    data.left = 0;
    data.right = 0;
    data.forward = 0;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(Data_package));
  }

  boomYValue = analogRead(AUX_VRX_PIN);
  boomXValue = analogRead(AUX_VRY_PIN);
  // Serial.println("yValue:"+ String(boomYValue));
  // Serial.println("xValue:"+ String(boomXValue));

  // check left/right press
  if (boomYValue >= UP_THRESHOLD) {
   // Serial.println("Boom up");
    data.up = 1;
    data.down = 0;
    data.rotateLeft = 0;
    data.rotateRight = 0;
    data.hookDown = 0;
    data.hookUp = 0;
    data.backward = 0;
    data.left = 0;
    data.right = 0;
    data.forward = 0;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(Data_package));

  } else if (boomYValue <= DOWN_THRESHOLD) {
   //Serial.println("boom down ");
    data.down = 1;
    data.up = 0;
    data.rotateLeft = 0;
    data.rotateRight = 0;
    data.hookDown = 0;
    data.hookUp = 0;
    data.backward = 0;
    data.left = 0;
    data.right = 0;
    data.forward = 0;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(Data_package));
  }

  // check up/donw press
  if (boomXValue <= LEFT_THRESHOLD) {
   //Serial.println("boom left");
    data.down = 0;
    data.up = 0;
    data.rotateLeft = 1;
    data.rotateRight = 0;
    data.hookDown = 0;
    data.hookUp = 0;
    data.backward = 0;
    data.left = 0;
    data.right = 0;
    data.forward = 0;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(Data_package));
  } else if (boomXValue >= RIGHT_THRESHOLD) {
   // Serial.println("boom right");
    data.down = 0;
    data.up = 0;
    data.rotateLeft = 0;
    data.rotateRight = 1;
    data.hookDown = 0;
    data.hookUp = 0;
    data.backward = 0;
    data.left = 0;
    data.right = 0;
    data.forward = 0;
    toggleSignalLED(HIGH);
    radio.write(&data, sizeof(Data_package));
  }

  // Send the whole data from the structure to the receiver
  // radio.write(&data, sizeof(Data_package));

  // reset all values:
  data.down = 0;
  data.up = 0;
  data.rotateLeft = 0;
  data.rotateRight = 0;
  data.hookDown = 0;
  data.hookUp = 0;
  data.backward = 0;
  data.left = 0;
  data.right = 0;
  data.forward = 0;
  toggleSignalLED(LOW);
  scanBatteryVoltage();
  delay(1);
}

void toggleSignalLED(byte state=LOW){
  digitalWrite(LED_SIGNAL, state);
}

void togglePowerLED( byte state=LOW){
  digitalWrite(LED_POWER, state);
}

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
        togglePowerLED(LOW);
      }else{
        togglePowerLED(HIGH);
      }
      ledState = !ledState;
    }else {
      // turn LED 
      togglePowerLED(HIGH);
    }
    lastTimeVoltageReading = millis();
   //Serial.println(volt);
  }
}