
#include "UnoJoy.h"

void setup(){
  setupPins();
  setupUnoJoy();
}

void loop(){
  // Always be getting fresh data
  dataForController_t controllerData = getControllerData();
  setControllerData(controllerData);
}

void setupPins(void){
  // Set all the digital pins as inputs
  // with the pull-up enabled, except for the 
  // two serial line pins
  for (int i = 2; i <= 13; i++){
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  }
  pinMode(1, INPUT);
  digitalWrite(1, HIGH);
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  pinMode(0, INPUT);
  digitalWrite(0, HIGH);
  pinMode(3, INPUT);
  digitalWrite(3, HIGH);
  pinMode(4, INPUT);
  digitalWrite(4, HIGH);
  pinMode(5, INPUT);
  digitalWrite(5, HIGH);
  pinMode(6, INPUT);
  digitalWrite(6, HIGH);
  pinMode(7, INPUT);
  digitalWrite(7, HIGH);
  pinMode(8, INPUT);
  digitalWrite(8, HIGH);
  pinMode(9, INPUT);
  digitalWrite(9, HIGH);
  pinMode(10, INPUT);
  digitalWrite(10, HIGH);
  pinMode(11, INPUT);
  digitalWrite(11, HIGH);
  pinMode(A4, INPUT);
  digitalWrite(A4, HIGH);
  pinMode(A5, INPUT);
  digitalWrite(A5, HIGH);
  
}

dataForController_t getControllerData(void){
  
  // Set up a place for our controller data
  //  Use the getBlankDataForController() function, since
  //  just declaring a fresh dataForController_t tends
  //  to get you one filled with junk from other, random
  //  values that were in those memory locations before
  dataForController_t controllerData = getBlankDataForController();
  // Since our buttons are all held high and
  //  pulled low when pressed, we use the "!"
  //  operator to invert the readings from the pins
  controllerData.squareOn = !digitalRead(A4);
  controllerData.triangleOn = !digitalRead(10);
  controllerData.circleOn = !digitalRead(8);
  controllerData.crossOn = !digitalRead(6);
  controllerData.dpadUpOn = !digitalRead(7);
  controllerData.dpadDownOn = !digitalRead(A5);
  controllerData.dpadLeftOn = !digitalRead(9);
  controllerData.dpadRightOn = !digitalRead(3);
  controllerData.r1On = !digitalRead(2);
  controllerData.r2On = !digitalRead(4);
  controllerData.l1On = !digitalRead(5);
  controllerData.l2On = !digitalRead(11);
  
  
  // Set the analog sticks
  //  Since analogRead(pin) returns a 10 bit value,
  //  we need to perform a bit shift operation to
  //  lose the 2 least significant bits and get an
  //  8 bit number that we can use  
  controllerData.rightStickX = analogRead(A0) >> 2;
  controllerData.rightStickY = analogRead(A3) >> 2;
  controllerData.leftStickX = analogRead(A1) >> 2;
  controllerData.leftStickY = analogRead(A2) >> 2;
  // And return the data!
  return controllerData;
  
}
