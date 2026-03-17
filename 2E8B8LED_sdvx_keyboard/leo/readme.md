# SDVX Keyboard Variant

This is the 8-button SDVX Leonardo build that can boot as either a USB gamepad or a USB keyboard/mouse.

This folder is the renamed successor to the older `2E8B8LED1RGB_sdvx/leo` variant. 

# What's New Compared With `2E8B8LED1RGB_sdvx`

1. RGB LED support was removed, so this build uses only the 8 single-color LEDs.
2. Button debounce was added with `Bounce2`.
3. A keyboard mode was added in addition to the original joystick/controller mode.
4. Keyboard mode can optionally keep the encoders active.
5. Keyboard mode now supports encoder output as keys or mouse wheel input.
6. Encoder handling was reworked with atomic reads, faster pin reads in interrupts, and normalized axis output.
7. Joystick reports are now only sent when buttons or encoder values actually change.
8. Keyboard mode has its own startup light pattern so you can tell which mode was entered.
9. HID light mode boot detection was fixed so `hidMode` is sampled correctly after mode selection.

# How To Use

Button order in code is:

`Start`, `BT-A`, `BT-B`, `BT-C`, `BT-D`, `FX-L`, `FX-R`, `Extra`

Boot mode selection:

1. Controller mode: plug in the board without holding `BT-A`.
2. Keyboard mode: hold `BT-A` while plugging in the board.
3. Keyboard mode with encoders disabled: hold `BT-A` and `BT-B` while plugging in the board.
4. Reactive LED mode: hold `Start` while plugging in the board.
5. HID LED mode with reactive fallback: plug in the board without holding `Start`.

Keep the mode-selection buttons held for about 1 second after USB connect, until startup begins.

Controller/keyboard selection and LED selection are independent, so you can combine them. For example, holding `BT-A` and `Start` enters keyboard mode with reactive LEDs.

Default keyboard mode mapping:

1. `BT-A` -> `Q`
2. `BT-B` -> `W`
3. `BT-C` -> `E`
4. `BT-D` -> `R`
5. `FX-L` -> `D`
6. `FX-R` -> `F`
7. Left encoder -> `Up` / `Down`
8. Right encoder -> mouse wheel

`Start` and `Extra` are not mapped to keyboard keys by default.

# Library Requirement

This sketch needs these Arduino libraries:

1. [ArduinoJoystickLibrary](https://github.com/MHeironimus/ArduinoJoystickLibrary/)
2. [Bounce2](https://github.com/thomasfredericks/Bounce2)

# How To Install And Import The Libraries

## ArduinoJoystickLibrary

1. Open the GitHub page: [ArduinoJoystickLibrary](https://github.com/MHeironimus/ArduinoJoystickLibrary/)
2. Click `Code` -> `Download ZIP`.
3. In Arduino IDE, open `Sketch` -> `Include Library` -> `Add .ZIP Library...`
4. Select the downloaded ZIP file and import it.

## Bounce2

Recommended method:

1. In Arduino IDE, open `Sketch` -> `Include Library` -> `Manage Libraries...`
2. Search for `Bounce2`
3. Click `Install`

ZIP import method:

1. Open the GitHub page: [Bounce2](https://github.com/thomasfredericks/Bounce2)
2. Click `Code` -> `Download ZIP`
3. In Arduino IDE, open `Sketch` -> `Include Library` -> `Add .ZIP Library...`
4. Select the downloaded ZIP file and import it

# Note

`HIDLED.h` and `HIDLED.cpp` are already included in this folder, so you do not need to install a separate LED library for this sketch.
