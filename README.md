# junomidi
JunoMidi expands the functionality of the Roland Juno Alpha 2 synthesizer to allow external modification of the parameters through the use of potentiometers. Allows multiple slide potentiometers to input into a Roland Alpha Juno 2 through MIDI and SysEx using an Arduino (Mega 2560). [Requires Libraries: MIDI, Adafruit GFX, 

## Features
JunoMidi was created with the intent of emulating the controls of the Roland Juno 106 on the Roland Alpha Juno 2. It is currently setup to handle 16 faders (as this is the max capacity of an Arduino Mega without muxing), with parameters in a similar order to the Juno 106's controls.

You can easily change which parameters are modified by the faders by changing the array or adding a new page to the array. 

#### OLED Screen Support
JunoMidi is configured for a 128x64 OLED screen. It will display the current parameter being modified and its value. It also has an idle screen and will display when a patch has been saved or loaded.

#### Saving and Loading Patches
This supports saving and loading patches to the Arduino itself using its EEPROM. It could potentially be configured with a (micro) SD card module in order to decrease wear on the EEPROM, but that would be relatively unnecessary due to the small size of the length 36 integer aray it stores for each patch. An SD card module could be a cool way to share and load patches made externally though. (EEPROM has a limit of 100,000 write cycles per single location)


## Instalation
Download the Arduino IDE
In the Library Manager, download "MIDI Library" https://github.com/FortySevenEffects/arduino_midi_library and "Adafruit GFX Library" https://github.com/adafruit/Adafruit-GFX-Library 
Make sure to set the board in settings to Arduino Mega

## Background
The Alpha Juno 2 suffers from poor design initiated by the design of the Yamaha DX7, where there were no analog controls and instead programming of the synth had to be done through a tiny screen. This project resolves this issue by restoring controls similar to Roland's previous synthesizers.

![Image of Alpha Juno 2](https://lh3.googleusercontent.com/proxy/bIYSk15KSz-GIXV04xa4mwdIBMIpHVlEffrRk_mwM3J3mVRDZoHoD3DW6tl1PI3df5t-eCp18HHImsXS7qIxy45C0R7t-_gIh_errUGwB6-qbUaHNrer-LA6-Z7siiQXfDAjg6PJE2ciIGbB)

JunoMidi in development
![JunoMidi in progress](https://cdn.discordapp.com/attachments/497557551140044850/856819422680842270/IMG_20210622_014203.jpg)
