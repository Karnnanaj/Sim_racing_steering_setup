#include <PicoGamepad.h>

PicoGamepad gamepad;

// Encoder
const uint8_t PIN_A = 2, PIN_B = 3, PIN_BTN = 4;
int32_t encoderPosition = 0, lastSentPosition = 0, centerOffset = 0;
int lastEncoded = 0;
const int stepSize = 4, maxSteps = 100;

// Buttons
const uint8_t pinA = 6, pinB = 7, pinX = 8, pinY = 9;
const uint8_t pinUp = 10, pinDown = 11, pinLeft = 12, pinRight = 13;
const uint8_t pinL1 = 14, pinR1 = 15;
const uint8_t analogPedal1 = 26, analogPedal2 = 27;
const uint8_t RESET_PIN = 21;

// Debouncing
const unsigned long debounceDelay = 10;
struct Button { uint8_t pin; bool lastState, stableState; unsigned long lastDebounceTime; };
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

// Calibration & LED blinking
bool calibrationMode = false;
unsigned long calibrationEntryTime = 0, calibrationExitTime = 0;
unsigned long ledBlinkLast = 0;
bool ledState = LOW;
const unsigned long entryHold = 2000, exitHold = 1000, blinkInterval = 500;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(PIN_BTN, INPUT_PULLUP);
  lastEncoded = (digitalRead(PIN_A) << 1) | digitalRead(PIN_B);

  for (int i = 0; i < numButtons; ++i)
    pinMode(buttons[i].pin, INPUT_PULLUP);

  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  unsigned long now = millis();

  // === Encoder quadrature decoding ===
  int MSB = digitalRead(PIN_A), LSB = digitalRead(PIN_B);
  int encoded = (MSB << 1) | LSB;
  int combined = (lastEncoded << 2) | encoded;
  const int8_t lookup[16] = {0, -1, 1, 0, 1,0,0,-1, -1,0,0,1, 0,1,-1,0};
  int delta = lookup[combined & 0x0F];
  if (delta) { encoderPosition += delta * stepSize; lastEncoded = encoded; }

  // Reset encoder center
  if (digitalRead(RESET_PIN) == LOW) encoderPosition = 0;

  // === Debounce digital buttons ===
  bool buttonChanged = false;
  for (int i = 0; i < numButtons; ++i) {
    bool r = digitalRead(buttons[i].pin);
    if (r != buttons[i].lastState) {
      buttons[i].lastDebounceTime = now;
      buttons[i].lastState = r;
    }
    if ((now - buttons[i].lastDebounceTime) >= debounceDelay) {
      if (r != buttons[i].stableState) {
        buttons[i].stableState = r;
        buttonChanged = true;
      }
    }
  }

  // === Calibration mode entry/exit & LED blinking ===
  bool gp12 = digitalRead(pinLeft) == LOW;
  bool gp14 = digitalRead(pinL1) == LOW;
  bool gp15 = digitalRead(pinR1) == LOW;
  bool gp10 = digitalRead(pinUp) == LOW;

  // Entry
  if (gp12 && gp14 && !calibrationMode) {
    if (!calibrationEntryTime) calibrationEntryTime = now;
    else if (now - calibrationEntryTime >= entryHold) {
      calibrationMode = true;
      Serial.println("Entered Calibration Mode");
      ledBlinkLast = now; ledState = HIGH;
      digitalWrite(LED_BUILTIN, ledState);
    }
  } else calibrationEntryTime = 0;

  // Exit
  if (calibrationMode && gp12 && gp14) {
    if (!calibrationExitTime) calibrationExitTime = now;
    else if (now - calibrationExitTime >= exitHold) {
      calibrationMode = false;
      Serial.println("Exited Calibration Mode");
      digitalWrite(LED_BUILTIN, LOW);
    }
  } else calibrationExitTime = 0;

  // Blink
  if (calibrationMode && (now - ledBlinkLast >= blinkInterval)) {
    ledBlinkLast = now;
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
  } else if (!calibrationMode) {
    digitalWrite(LED_BUILTIN, LOW);
  }

  // === Adjust center in calibration ===
  if (calibrationMode) {
    if (gp15) { centerOffset += stepSize; delay(200); }
    if (gp10) { centerOffset -= stepSize; delay(200); }
  }

  // === Send gamepad report ===
  static unsigned long lastSend = 0;
  if ((encoderPosition + centerOffset) != lastSentPosition || buttonChanged || (now - lastSend >= 5)) {
    int32_t c = constrain(encoderPosition + centerOffset, -maxSteps, maxSteps);
    gamepad.SetX(map(c, -maxSteps, maxSteps, -32767, 32767));

    for (int i = 0; i < 11; ++i)
      gamepad.SetButton(i, !buttons[i].stableState);

    int v1 = analogRead(analogPedal1), v2 = analogRead(analogPedal2);
    gamepad.SetRx(map(v1, 0, 4095, 0, 65535));
    gamepad.SetRy(map(v2, 0, 4095, 0, 65535));

    gamepad.send_update();
    lastSentPosition = c;
    lastSend = now;
    // toggle LED to show activity (optional)
  }
}
