#include "UnoJoy.h"

// Define pins for rotary encoder and center position switch
const int encoderPinA = 2;   // Rotary encoder pin A
const int encoderPinB = 3;   // Rotary encoder pin B
const int centerSwitchPin = 4; // Pin for the center position switch

int encoderValue = 127;      // Initialize the value to the center position (0-255 for 8-bit analog)
int lastEncoderStateA = LOW; // Track the last state of pin A

void setup() {
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  pinMode(centerSwitchPin, INPUT_PULLUP);  // Center switch pin
  
  setupPins();
  setupUnoJoy();
}

void loop() {
  dataForController_t controllerData = getControllerData();
  setControllerData(controllerData);

  // Check if the center switch is pressed to reset encoder value
  if (digitalRead(centerSwitchPin) == LOW) {
    encoderValue = 127;  // Reset to center position
  } else {
    // Read the rotary encoder and update steering input if not at center
    updateSteeringWithEncoder();
  }
}

void setupPins(void) {
  for (int i = 2; i <= 13; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
}

dataForController_t getControllerData(void) {
  dataForController_t controllerData = getBlankDataForController();
  controllerData.leftStickX = encoderValue;

  controllerData.squareOn = !digitalRead(A4);
  controllerData.triangleOn = !digitalRead(10);
  controllerData.circleOn = !digitalRead(8);
  controllerData.crossOn = !digitalRead(6);
  controllerData.dpadUpOn = !digitalRead(7);
  controllerData.dpadDownOn = !digitalRead(A5);
  controllerData.dpadLeftOn = !digitalRead(9);
  controllerData.r1On = !digitalRead(5);
  controllerData.r2On = !digitalRead(4);
  controllerData.l1On = !digitalRead(5);
  controllerData.l2On = !digitalRead(11);

  controllerData.leftStickY = analogRead(A1) >> 2;
  controllerData.rightStickX = analogRead(A0) >> 2;
  controllerData.rightStickY = analogRead(A2) >> 2;

  return controllerData;
}

void updateSteeringWithEncoder() {
  int currentEncoderStateA = digitalRead(encoderPinA);

  // Debounce and check for a change in state
  if (currentEncoderStateA != lastEncoderStateA) {
    if (digitalRead(encoderPinB) != currentEncoderStateA) {
      encoderValue = min(255, encoderValue + 1);  // Clockwise rotation
    } else {
      encoderValue = max(0, encoderValue - 1);    // Counterclockwise rotation
    }
    lastEncoderStateA = currentEncoderStateA;
  }
}
