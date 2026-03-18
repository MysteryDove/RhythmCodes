# RhythmCodes – Project Guidelines

Arduino Leonardo (ATmega32U4) rhythm game controller firmware collection. Each subfolder targets a specific game (IIDX, SDVX, Pop'n, Museca, etc.) with varying encoder/button/LED counts.

## Architecture

- **Folder naming**: `[EncCount]E[ButtonCount]B[LEDCount]LED_[game]/leo/`
- **Per-project files**: `leo.ino` (main sketch), `HIDLED.cpp` + `HIDLED.h` (USB HID LED output reports)
- **HIDLED library** is copy-pasted per project (not yet centralized). Derived from [mon/Arduino-HID-Lighting](https://github.com/mon/Arduino-HID-Lighting). When editing, check if the change should propagate to other project copies.
- **GameController (gcjoy/gckb)** variants do NOT use HIDLED — they use SwitchControlLibrary or Keyboard.h directly.

## Code Style

- C/C++ Arduino style; no `.clang-format` — follow existing formatting in each file.
- Pin arrays are always `byte[]` with `sizeof(arr)/sizeof(arr[0])` for iteration.
- Constants: `UPPER_CASE` for `#define`, `PascalCase` for globals (`ReportDelay`, `ReactiveTimeoutMax`).
- Prefer `digitalRead`/`digitalWrite`; use `analogWrite` only for PWM LED brightness in HIDLED.

## Conventions

- **Buttons**: `INPUT_PULLUP`, active LOW (grounded = pressed).
- **LEDs**: Active HIGH, direct-driven with external resistor.
- **Encoders**: Quadrature decode via `attachInterrupt(..., CHANGE)`. Typically `PULSE = 600`.
- **Boot mode detection**: Read button state at startup; held button = alternate mode. Blink LEDs at 500 ms while waiting for release.
- **Two LED modes in every project**:
  - *Reactive* (`hidMode == false`): LEDs mirror inverted button state.
  - *HID* (`hidMode == true`): Host controls LEDs via USB HID output reports; falls back to reactive after `ReactiveTimeoutMax` cycles without updates.
- **Report timing**: `delayMicroseconds(ReportDelay)` with `ReportDelay = 700`.
- **Joystick**: `Joystick_.begin(false)` (manual send); call `Joystick_.sendState()` explicitly.
- **Serial**: `Serial.begin(9600)` for debug output.

## Build & Test

- **Board**: Arduino Leonardo (Arduino AVR Boards package).
- **IDE**: Arduino IDE 1.8.x+ or Arduino CLI.
- **Required libraries** (install manually):
  - [ArduinoJoystickLibrary](https://github.com/MHeironimus/ArduinoJoystickLibrary) — most joystick projects
  - [ArduinoJoystickLibrary (forked)](https://github.com/MysteryDove/ArduinoJoystickLibrary) — `2E8B8LED_sdvx_keyboard` (adds `isReady()` for non-blocking send)
  - [Bounce2](https://github.com/thomasfredericks/Bounce2) — `2E8B8LED_sdvx_keyboard` only
  - [SwitchControlLibrary](https://github.com/celclow/SwitchControlLibrary) — `2J3B3LED_gcjoy` only
- **No automated tests** — verification is done by flashing to hardware.
- Compile example: `arduino-cli compile --fqbn arduino:avr:leonardo 2E10B10LED_sdvx/leo/`

## Known TODOs (from README)

- Implement debounce library across all projects (currently only sdvx_keyboard)
- Centralize HIDLED into a proper Arduino library
- Mixed light mode (reactive + HID blend)
- EEPROM for persistent user settings
