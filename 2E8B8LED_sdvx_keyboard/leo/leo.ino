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
#include <Joystick.h>
#include <Bounce2.h>
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD, 8, 0,
                   true, true, false, false, false, false, false, false, false, false, false);

boolean hidMode, state[2] = { false }, set[4] = { false };
int encL = 0, encR = 0;
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

unsigned long ReportRate;
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
  if (digitalRead(ButtonPins[1]) == LOW) {
    Joystick.begin(false);
    Joystick.setXAxisRange(-PULSE / 2, PULSE / 2 - 1);
    Joystick.setYAxisRange(-PULSE / 2, PULSE / 2 - 1);
    attachInterrupt(digitalPinToInterrupt(EncPins[0]), doEncoder0, CHANGE);
    attachInterrupt(digitalPinToInterrupt(EncPins[2]), doEncoder1, CHANGE);
  } else {
    controllerMode = false;
    Serial.println("KB mode active!");
    Keyboard.begin();
  }
  // light mode detection
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
    for (int i = 0; i < ButtonCount; i++) {
      buttons[i].update();
      if (buttons[i].fell() || buttons[i].rose()) {  // Button was just pressed
        Joystick.setButton(i, !(buttons[i].read()));
      }
    }
    //read encoders, detect overflow and rollover
    encL %= PULSE;
    if (encL < -PULSE / 2) encL += PULSE;
    if (encL > PULSE / 2 - 1) encL -= PULSE;

    encR %= PULSE;
    if (encR < -PULSE / 2) encR += PULSE;
    if (encR > PULSE / 2 - 1) encR -= PULSE;
    Joystick.setXAxis(encL);
    Joystick.setYAxis(encR);
    //report
    Joystick.sendState();
  } else {
    // keyboard mode
    const char keyMapping[] = { 0, 'q', 'w', 'e', 'r', 'd', 'f', 0 };  // Map buttons to keys
    for (int i = 0; i < ButtonCount; i++) {
      buttons[i].update();
      if (buttons[i].fell()) {  // Button was just pressed
        Keyboard.press(keyMapping[i]);
      } else if (buttons[i].rose()) {  // Button was just released
        Keyboard.release(keyMapping[i]);
      }
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
  // Serial.print(micros() - loopStartTime);
  // Serial.println(" micro sec per loop");
}
//Interrupts
void doEncoder0() {
  if (state[0] == false && digitalRead(EncPins[0]) == LOW) {
    set[0] = digitalRead(EncPins[1]);
    state[0] = true;
  }
  if (state[0] == true && digitalRead(EncPins[0]) == HIGH) {
    set[1] = !digitalRead(EncPins[1]);
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
  if (state[1] == false && digitalRead(EncPins[2]) == LOW) {
    set[2] = digitalRead(EncPins[3]);
    state[1] = true;
  }
  if (state[1] == true && digitalRead(EncPins[2]) == HIGH) {
    set[3] = !digitalRead(EncPins[3]);
    if (set[2] == true && set[3] == true) {
      encR++;
    }
    if (set[2] == false && set[3] == false) {
      encR--;
    }
    state[1] = false;
  }
}
