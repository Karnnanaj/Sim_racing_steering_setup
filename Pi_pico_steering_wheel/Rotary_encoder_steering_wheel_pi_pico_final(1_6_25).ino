#include <PicoGamepad.h>

PicoGamepad gamepad;

// Rotary encoder
const uint8_t PIN_A = 2, PIN_B = 3, PIN_BTN = 4;
int32_t encoderPosition = 0, lastSentPosition = 0;
int lastEncoded = 0;
const int stepSize = 4;
const int maxSteps = 100;
int32_t centerOffset = 0;

// Buttons
const uint8_t pinA = 6, pinB = 7, pinX = 8, pinY = 9;
const uint8_t pinUp = 10, pinDown = 11, pinLeft = 12, pinRight = 13;
const uint8_t pinL1 = 14, pinR1 = 15;

// Analog inputs
const uint8_t analogPedal1 = 26;
const uint8_t analogPedal2 = 27;

// Reset pin for centering encoder
#define RESET_PIN 21

// Debouncing parameters
const unsigned long debounceDelay = 10; // Reduced to 10ms for responsiveness
struct Button {
  uint8_t pin;
  bool lastState;
  bool stableState;
  unsigned long lastDebounceTime;
};

// Array of buttons to debounce
Button buttons[] = {
  {PIN_BTN, HIGH, HIGH, 0},
  {pinA, HIGH, HIGH, 0},
  {pinB, HIGH, HIGH, 0},
  {pinX, HIGH, HIGH, 0},
  {pinY, HIGH, HIGH, 0},
  {pinUp, HIGH, HIGH, 0},
  {pinDown, HIGH, HIGH, 0},
  {pinLeft, HIGH, HIGH, 0},
  {pinRight, HIGH, HIGH, 0},
  {pinL1, HIGH, HIGH, 0},
  {pinR1, HIGH, HIGH, 0}
};
const int numButtons = sizeof(buttons) / sizeof(buttons[0]);

// Calibration mode variables
bool calibrationMode = false;
unsigned long calibrationEntryTime = 0;
unsigned long calibrationExitTime = 0;

void setup() {
  Serial.begin(115200);

  // Initialize encoder pins
  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(PIN_BTN, INPUT_PULLUP);
  lastEncoded = (digitalRead(PIN_A) << 1) | digitalRead(PIN_B);

  // Initialize button pins
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }

  // Initialize reset pin
  pinMode(RESET_PIN, INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // === Encoder ===
  int MSB = digitalRead(PIN_A);
  int LSB = digitalRead(PIN_B);
  int encoded = (MSB << 1) | LSB;
  int combined = (lastEncoded << 2) | encoded;

  const int8_t encoderLookupTable[16] = {
    0, -1,  1,  0,
    1,  0,  0, -1,
   -1,  0,  0,  1,
    0,  1, -1,  0
  };

  int delta = encoderLookupTable[combined & 0x0F];
  if (delta != 0) {
    encoderPosition += delta * stepSize;
    lastEncoded = encoded;
  }

  // === Reset encoder to center ===
  if (digitalRead(RESET_PIN) == LOW) {
    encoderPosition = 0;
  }

  // === Debounce Buttons ===
  bool buttonChanged = false;
  for (int i = 0; i < numButtons; i++) {
    bool reading = digitalRead(buttons[i].pin);
    if (reading != buttons[i].lastState) {
      buttons[i].lastDebounceTime = millis();
      buttons[i].lastState = reading;
    }
    if ((millis() - buttons[i].lastDebounceTime) >= debounceDelay) {
      if (reading != buttons[i].stableState) {
        buttons[i].stableState = reading;
        buttonChanged = true;
      }
    }
  }

  // === Calibration Mode Handling ===
  bool gp12Pressed = digitalRead(pinLeft) == LOW;
  bool gp14Pressed = digitalRead(pinL1) == LOW;
  bool gp15Pressed = digitalRead(pinR1) == LOW;
  bool gp10Pressed = digitalRead(pinUp) == LOW;

  unsigned long currentTime = millis();

  // Check for calibration mode entry
  if (gp12Pressed && gp14Pressed) {
    if (calibrationEntryTime == 0) {
      calibrationEntryTime = currentTime;
    } else if (!calibrationMode && (currentTime - calibrationEntryTime >= 5000)) {
      calibrationMode = true;
      Serial.println("Entered Calibration Mode");
    }
  } else {
    calibrationEntryTime = 0;
  }

  // Check for calibration mode exit
  if (calibrationMode && gp12Pressed && gp14Pressed) {
    if (calibrationExitTime == 0) {
      calibrationExitTime = currentTime;
    } else if (currentTime - calibrationExitTime >= 2000) {
      calibrationMode = false;
      Serial.println("Exited Calibration Mode");
    }
  } else {
    calibrationExitTime = 0;
  }

  // Adjust center offset in calibration mode
  if (calibrationMode) {
    if (gp15Pressed) {
      centerOffset += stepSize;
      delay(200); // Debounce delay
    }
    if (gp10Pressed) {
      centerOffset -= stepSize;
      delay(200); // Debounce delay
    }
  }

  // === Gamepad Update ===
  static uint32_t lastSend = 0;
  uint32_t now = millis();

  if ((encoderPosition + centerOffset) != lastSentPosition || buttonChanged || (now - lastSend >= 5)) {
    int32_t clamped = constrain(encoderPosition + centerOffset, -maxSteps, maxSteps);
    int16_t hidX = map(clamped, -maxSteps, maxSteps, -32767, 32767);

    gamepad.SetX(hidX);
    // Set buttons (inverted logic: LOW = pressed)
    gamepad.SetButton(0, !buttons[0].stableState); // PIN_BTN
    gamepad.SetButton(1, !buttons[1].stableState); // pinA
    gamepad.SetButton(2, !buttons[2].stableState); // pinB
    gamepad.SetButton(3, !buttons[3].stableState); // pinX
    gamepad.SetButton(4, !buttons[4].stableState); // pinY
    gamepad.SetButton(5, !buttons[5].stableState); // pinL1
    gamepad.SetButton(6, !buttons[6].stableState); // pinR1
    gamepad.SetButton(7, !buttons[7].stableState); // pinUp
    gamepad.SetButton(8, !buttons[8].stableState); // pinDown
    gamepad.SetButton(9, !buttons[9].stableState); // pinLeft
    gamepad.SetButton(10, !buttons[10].stableState); // pinRight

    // Read analog pedals (full 0-4095 range)
    int val1 = analogRead(analogPedal1);
    int val2 = analogRead(analogPedal2);

    // Map to 0-65535 for gamepad axes
    gamepad.SetRx(map(val1, 0, 4095, 0, 65535));
    gamepad.SetRy(map(val2, 0, 4095, 0, 65535));

    gamepad.send_update();

    lastSentPosition = encoderPosition + centerOffset;
    lastSend = now;
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}
