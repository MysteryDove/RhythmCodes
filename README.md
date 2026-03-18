# RhythmCodes

Forked version of `RhythmCodes` for DIY rhythm game controllers based on Arduino Leonardo.

Current focus: SDVX controller code, especially the maintained variants under `2E8B8LED_sdvx_keyboard` and the related SDVX folders in this repository.

## Dependencies

Most projects use the upstream [ArduinoJoystickLibrary](https://github.com/MHeironimus/ArduinoJoystickLibrary/).

`2E8B8LED_sdvx_keyboard` requires the [forked ArduinoJoystickLibrary](https://github.com/MysteryDove/ArduinoJoystickLibrary/) which adds a non-blocking `isReady()` endpoint check for constant-rate USB HID reporting.

# Todo list:
* implement debounce library
* librarylize HID light code
* mixed light mode
* eeprom function to keep user settings
