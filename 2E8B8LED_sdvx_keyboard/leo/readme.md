# Lib Requirement
This Arduino code requires:
[ArduinoJoyStickLibrary](https://github.com/MHeironimus/ArduinoJoystickLibrary/): you need to download as zip and import into Arduino IDE.

[Bounce2](https://github.com/thomasfredericks/Bounce2): you can directly download this library using arduino

# What changed
1. Button debounce implemented using Bounce2
2. RGB LED support removed because personally do not using that
3. Using dynamic report delay to mitigate unstable report rate caused by the encoder interrupts
4. Adding keyboard mode so you can play djmax respect 4key, osu mania 4k/6k and even league of legends!
5. Added keyboard start light effect when starting in keyboard mode
