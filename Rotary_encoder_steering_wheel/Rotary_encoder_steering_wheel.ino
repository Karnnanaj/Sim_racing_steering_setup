#include "UnoJoy.h"

// Define pins for rotary encoder
const int encoderPinA = 2;   // Rotary encoder pin A
const int encoderPinB = 3;   // Rotary encoder pin B

int encoderValue = 127;      // Initialize the value to the center position (0-255 for 8-bit analog)
int lastEncoderStateA = LOW; // Track the last state of pin A

void setup() {
  // Initialize encoder pins as inputs
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  // Enable pull-up resistors for encoder pins (if needed)
  digitalWrite(encoderPinA, HIGH);
  digitalWrite(encoderPinB, HIGH);

  setupPins();
  setupUnoJoy();
}

void loop() {
  // Always be getting fresh data
  dataForController_t controllerData = getControllerData();
  setControllerData(controllerData);

  // Read the rotary encoder and update steering input
  updateSteeringWithEncoder();
}

void setupPins(void) {
  // Set all the digital pins as inputs with the pull-up enabled, except for the two serial line pins
  for (int i = 2; i <= 13; i++) {
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  }
  pinMode(A4, INPUT);
  digitalWrite(A4, HIGH);
  pinMode(A5, INPUT);
  digitalWrite(A5, HIGH);
}

dataForController_t getControllerData(void) {
  // Get blank data for controller
  dataForController_t controllerData = getBlankDataForController();

  // Map the encoder value to the right stick Y (steering wheel)
  controllerData.rightStickY = encoderValue;

  // Set other buttons as per your original sketch
  controllerData.squareOn = !digitalRead(A4);
  controllerData.triangleOn = !digitalRead(10);
  controllerData.circleOn = !digitalRead(8);
  controllerData.crossOn = !digitalRead(6);
  controllerData.dpadUpOn = !digitalRead(7);
  controllerData.dpadDownOn = !digitalRead(A5);
  controllerData.dpadLeftOn = !digitalRead(9);
//  controllerData.dpadRightOn = !digitalRead(3);
  controllerData.r1On = !digitalRead(5);
  controllerData.r2On = !digitalRead(4);
  controllerData.l1On = !digitalRead(5);
  controllerData.l2On = !digitalRead(11);

controllerData.leftStickX = analogRead(A1) >> 2;
controllerData.rightStickX = analogRead(A0) >> 2;
controllerData.leftStickY = analogRead(A2) >> 2;
  return controllerData;
}

// Function to read the rotary encoder and update the steering input
void updateSteeringWithEncoder() {
  // Read the current state of encoder pin A
  int currentEncoderStateA = digitalRead(encoderPinA);

  // Check for a change in state of pin A (indicating rotation)
  if (currentEncoderStateA != lastEncoderStateA) {
    // Check the state of pin B to determine rotation direction
    if (digitalRead(encoderPinB) != currentEncoderStateA) {
      // Rotating clockwise, increase value
      encoderValue = min(255, encoderValue + 1);
    } else {
      // Rotating counterclockwise, decrease value
      encoderValue = max(0, encoderValue - 1);
    }
  }

  // Save the current state as the last state for the next loop
  lastEncoderStateA = currentEncoderStateA;
}
