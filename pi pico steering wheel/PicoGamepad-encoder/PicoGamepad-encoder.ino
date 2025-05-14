#include <PicoGamepad.h>

PicoGamepad gamepad;

const uint PIN_A = 2;
const uint PIN_B = 3;
const uint PIN_BTN = 4;

int32_t encoderPosition = 0;
int32_t lastSentPosition = 0;

int lastEncoded = 0;
const int stepSize = 4;           // Adjust this to control sensitivity
const int maxSteps = 100;

void setup() {
  Serial.begin(115200);

  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  lastEncoded = (digitalRead(PIN_A) << 1) | digitalRead(PIN_B);
}

void loop() {
  // === FAST POLLING ENCODER ===
  int MSB = digitalRead(PIN_A);
  int LSB = digitalRead(PIN_B);
  int encoded = (MSB << 1) | LSB;
  int combined = (lastEncoded << 2) | encoded;

  // Lookup table for direction
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

  // === GAMEPAD UPDATE EVERY ~5ms ===
  static uint32_t lastSend = 0;
  uint32_t now = millis();
  if ((encoderPosition != lastSentPosition) || (now - lastSend >= 5)) {
    int32_t clamped = constrain(encoderPosition, -maxSteps, maxSteps);
    int16_t hidX = map(clamped, -maxSteps, maxSteps, -32767, 32767);

    gamepad.SetX(hidX);
    gamepad.SetButton(0, !digitalRead(PIN_BTN));
    gamepad.send_update();

    lastSentPosition = encoderPosition;
    lastSend = now;
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}
