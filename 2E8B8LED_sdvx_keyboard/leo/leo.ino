/*Arduino SDVX Controller Code for Leonardo
 * 2 Encoders + 8 Buttons + 11 HID controlable LED
 * release page
 * http://knuckleslee.blogspot.com/2018/06/RhythmCodes.html
 * 
 * Arduino Joystick Library
 * https://github.com/MHeironimus/ArduinoJoystickLibrary/
 * mon's Arduino-HID-Lighting
 * https://github.com/mon/Arduino-HID-Lighting
 */
#include <Keyboard.h>
#include <Mouse.h>
#include <Joystick.h>
#include <Bounce2.h>
#include <util/atomic.h>
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD, 8, 0,
                   true, true, false, false, false, false, false, false, false, false, false);

boolean hidMode;
volatile bool state[2] = { false }, set[4] = { false };
volatile int32_t encL = 0, encR = 0;
const int PULSE = 600;  //number of pulses per revolution of encoders
byte EncPins[] = { 0, 1, 2, 3 };
// button sorting = {Start,BT-A,-B,-C,-D,FX-L,-R,Extra}
byte SinglePins[] = { 4, 6, 12, 18, 20, 22, 14, 16 };
byte ButtonPins[] = { 5, 7, 13, 19, 21, 23, 15, 17 };
unsigned long ReactiveTimeoutMax = 1000;
/* pin assignments
 * VOL-L Green to pin 0 and White to pin 1
 * VOL-R Green to pin 2 and White to pin 3
 * current pin layout
 *  SinglePins {4, 6, 12,18,20,22,14,16} = LED 1 to 8
 *    connect pin to resistor and then + termnial of LED
 *    connect ground to - terminal of LED
 *  ButtonPins {5, 7, 13,19,21,23,15,17} = Button input 1 to 8
 *    connect button pin to ground to trigger button press
 *  Light mode detection by read first button while connecting usb 
 *   hold    = false = reactive lighting 
 *   release = true  = HID lighting with reactive fallback
 */
const byte ButtonCount = sizeof(ButtonPins) / sizeof(ButtonPins[0]);
const byte SingleCount = sizeof(SinglePins) / sizeof(SinglePins[0]);
const byte EncPinCount = sizeof(EncPins) / sizeof(EncPins[0]);
unsigned long ReactiveTimeoutCount = ReactiveTimeoutMax;
Bounce buttons[ButtonCount];
bool controllerMode = true;

enum EncoderOutputMode {
  ENCODER_MODE_KEYS = 0,
  ENCODER_MODE_WHEEL = 1
};

// Per-encoder output configuration (keyboard mode).
// 0 = left encoder, 1 = right encoder
EncoderOutputMode encoderOutput[2] = {
  ENCODER_MODE_KEYS,
  ENCODER_MODE_WHEEL
};

// Encoder keybinds (keyboard mode). Customize if needed.
// Indexing: [encoder][direction], direction: 0 = CCW, 1 = CW
const uint8_t EncoderKeybinds[2][2] = {
  { KEY_UP_ARROW, KEY_DOWN_ARROW },  // Left encoder
  { KEY_UP_ARROW, KEY_DOWN_ARROW }   // Right encoder
};

const int8_t EncoderWheelStep = 1;  // Wheel ticks per detent
// Encoder slowdown/deadzone for keyboard mode.
// Every N encoder moves (detents) will emit 1 output step.
// 1 = no slowdown; higher values = more deadzone + lower rate.
const uint8_t EncoderStepDiv[2] = { 20, 20 };
// Enable/disable encoder input in keyboard mode (performance).
// false = disable encoder processing in keyboard mode.
bool EnableEncoderInKeyboardMode = false;

unsigned long ReportRate;
void handleEncodersKeyboardMode();
void tapKey(uint8_t key);

static inline uint8_t fastRead(uint8_t pin) {
  uint8_t port = digitalPinToPort(pin);
  if (port == NOT_A_PIN) {
    return LOW;
  }
  return ((*portInputRegister(port)) & digitalPinToBitMask(pin)) ? HIGH : LOW;
}

static inline int32_t atomicRead32(volatile int32_t* value) {
  int32_t copy;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    copy = *value;
  }
  return copy;
}

static inline int16_t normalizeEncoder(int32_t value) {
  value %= PULSE;
  if (value < -PULSE / 2) value += PULSE;
  if (value > PULSE / 2 - 1) value -= PULSE;
  return (int16_t)value;
}
void setup() {
  Serial.begin(9600);
  // setup I/O for pins
  hidMode = digitalRead(ButtonPins[0]);
  for (int i = 0; i < ButtonCount; i++) {
    buttons[i] = Bounce();
    buttons[i].attach(ButtonPins[i], INPUT_PULLUP);
    buttons[i].interval(5);
  }
  for (int i = 0; i < SingleCount; i++) {
    pinMode(SinglePins[i], OUTPUT);
  }
  for (int i = 0; i < EncPinCount; i++) {
    pinMode(EncPins[i], INPUT_PULLUP);
  }
  delay(1000);
  // If not holding BT-A when plugged, go to controller mode, otherwise will go to keyboard mode
  if (digitalRead(ButtonPins[1]) == HIGH) {
    Joystick.begin(false);
    Joystick.setXAxisRange(-PULSE / 2, PULSE / 2 - 1);
    Joystick.setYAxisRange(-PULSE / 2, PULSE / 2 - 1);
    attachInterrupt(digitalPinToInterrupt(EncPins[0]), doEncoder0, CHANGE);
    attachInterrupt(digitalPinToInterrupt(EncPins[2]), doEncoder1, CHANGE);
  } else {
    controllerMode = false;
    Serial.println("KB mode active!");
    Serial.println(digitalRead(ButtonPins[2]));
    if(digitalRead(ButtonPins[2]) == HIGH){
      Serial.println("Encoder in KB mode enabled!");
      EnableEncoderInKeyboardMode = true;
      attachInterrupt(digitalPinToInterrupt(EncPins[0]), doEncoder0, CHANGE);
      attachInterrupt(digitalPinToInterrupt(EncPins[2]), doEncoder1, CHANGE);
    } else {
      Serial.println("Encoder in KB mode disabled!");
    }
    Keyboard.begin();
    Mouse.begin();
  }
  // Encoder interrupts active in both modes

  // light mode detection
  hidMode = digitalRead(ButtonPins[0]);
  while (digitalRead(ButtonPins[0]) == LOW) {
    if ((millis() % 1000) < 500) {
      for (int i = 0; i < SingleCount; i++) {
        digitalWrite(SinglePins[i], HIGH);
      }
    } else if ((millis() % 1000) > 500) {
      for (int i = 0; i < SingleCount; i++) {
        digitalWrite(SinglePins[i], LOW);
      }
    }
  }
  for (int i = 0; i < SingleCount; i++) {
    digitalWrite(SinglePins[i], LOW);
  }
  //boot light
  if (controllerMode) {
    int startup[] = { 0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1, 0, 1, 5, 2, 0, 3, 6, 4, 6, 3, 0, 2, 5, 1 };
    for (int i = 0; i < (sizeof(startup) / sizeof(startup[0])); i++) {
      digitalWrite(SinglePins[startup[i]], HIGH);
      delay(80);
      digitalWrite(SinglePins[startup[i]], LOW);
    }
    for (int i = 0; i < SingleCount; i++) {
      digitalWrite(SinglePins[i], HIGH);
    }
    delay(500);
    for (int i = 0; i < SingleCount; i++) {
      digitalWrite(SinglePins[i], LOW);
    }
  } else {
    // Use different boot light to note user we are in kb mode
    int startup[] = { 0, 1, 2, 3, 4, 5, 6 };
    for (int i = 0; i < (sizeof(startup) / sizeof(startup[0])); i++) {
      digitalWrite(SinglePins[startup[i]], HIGH);
    }
    delay(500);
    for (int i = 0; i < (sizeof(startup) / sizeof(startup[0])); i++) {
      digitalWrite(SinglePins[startup[i]], LOW);
    }
    delay(500);
    for (int i = 0; i < (sizeof(startup) / sizeof(startup[0])); i++) {
      digitalWrite(SinglePins[startup[i]], HIGH);
    }
    delay(500);
    for (int i = 0; i < (sizeof(startup) / sizeof(startup[0])); i++) {
      digitalWrite(SinglePins[startup[i]], LOW);
    }
  }
}  //end setup

void loop() {
  unsigned long loopStartTime = micros();  // Record the start time
  if (controllerMode) {
    bool changed = false;
    static int16_t lastSentX = 0;
    static int16_t lastSentY = 0;

    for (int i = 0; i < ButtonCount; i++) {
      buttons[i].update();
      if (buttons[i].fell() || buttons[i].rose()) {  // Button was just pressed
        Joystick.setButton(i, !(buttons[i].read()));
        changed = true;
      }
    }

    int16_t x = normalizeEncoder(atomicRead32(&encL));
    int16_t y = normalizeEncoder(atomicRead32(&encR));
    if (x != lastSentX || y != lastSentY) {
      Joystick.setXAxis(x);
      Joystick.setYAxis(y);
      lastSentX = x;
      lastSentY = y;
      changed = true;
    }

    if (changed) {
      Joystick.sendState();
    }
  } else {
    // keyboard mode
    const char keyMapping[] = { 0, 'q', 'w', 'e', 'r', 'd', 'f', 0 };  // Map buttons to keys
    for (int i = 0; i < ButtonCount; i++) {
      buttons[i].update();
      if (buttons[i].fell()) {  // Button was just pressed
        if (keyMapping[i] != 0) {
          Keyboard.press(keyMapping[i]);
        }
      } else if (buttons[i].rose()) {  // Button was just released
        if (keyMapping[i] != 0) {
          Keyboard.release(keyMapping[i]);
        }
      }
    }
    if (EnableEncoderInKeyboardMode) {
      handleEncodersKeyboardMode();
    }
  }
  if (!hidMode || ReactiveTimeoutCount >= ReactiveTimeoutMax) {
    for (int i = 0; i < ButtonCount; i++) {
      digitalWrite(SinglePins[i], !(digitalRead(ButtonPins[i])));
    }
  } else if (hidMode) {
    ReactiveTimeoutCount++;
  }

  unsigned long loopTime = micros() - loopStartTime;
  if (loopTime < 1000) {
    delayMicroseconds(1000 - loopTime);
  }
  //ReportRate Display
  //Serial.print(micros() - loopStartTime);
  //Serial.println(" micro sec per loop");
}

inline void emitEncoderStep(uint8_t index, bool cw) {
  if (encoderOutput[index] == ENCODER_MODE_WHEEL) {
    Mouse.move(0, 0, cw ? -EncoderWheelStep : EncoderWheelStep);
  } else {
    tapKey(EncoderKeybinds[index][cw ? 1 : 0]);
  }
}

void handleEncodersKeyboardMode() {
  static int lastEncL = 0;
  static int lastEncR = 0;
  static int accumL = 0;
  static int accumR = 0;

  int currentEncL = (int)atomicRead32(&encL);
  int currentEncR = (int)atomicRead32(&encR);

  int deltaL = currentEncL - lastEncL;
  int deltaR = currentEncR - lastEncR;

  if (deltaL != 0) {
    accumL += deltaL;
    uint8_t threshold = EncoderStepDiv[0] == 0 ? 1 : EncoderStepDiv[0];
    while (accumL >= threshold) {
      emitEncoderStep(0, true);
      accumL -= threshold;
    }
    while (accumL <= -threshold) {
      emitEncoderStep(0, false);
      accumL += threshold;
    }
  }

  if (deltaR != 0) {
    accumR += deltaR;
    uint8_t threshold = EncoderStepDiv[1] == 0 ? 1 : EncoderStepDiv[1];
    while (accumR >= threshold) {
      emitEncoderStep(1, true);
      accumR -= threshold;
    }
    while (accumR <= -threshold) {
      emitEncoderStep(1, false);
      accumR += threshold;
    }
  }

  lastEncL = currentEncL;
  lastEncR = currentEncR;
}

void tapKey(uint8_t key) {
  if (key == 0) {
    return;
  }
  Keyboard.press(key);
  Keyboard.release(key);
}
//Interrupts
void doEncoder0() {
  if (state[0] == false && fastRead(EncPins[0]) == LOW) {
    set[0] = fastRead(EncPins[1]);
    state[0] = true;
  }
  if (state[0] == true && fastRead(EncPins[0]) == HIGH) {
    set[1] = !fastRead(EncPins[1]);
    if (set[0] == true && set[1] == true) {
      encL++;
    }
    if (set[0] == false && set[1] == false) {
      encL--;
    }
    state[0] = false;
  }
}

void doEncoder1() {
  if (state[1] == false && fastRead(EncPins[2]) == LOW) {
    set[2] = fastRead(EncPins[3]);
    state[1] = true;
  }
  if (state[1] == true && fastRead(EncPins[2]) == HIGH) {
    set[3] = !fastRead(EncPins[3]);
    if (set[2] == true && set[3] == true) {
      encR++;
    }
    if (set[2] == false && set[3] == false) {
      encR--;
    }
    state[1] = false;
  }
}
