# Mobiflight-TFT-Instruments

## Set up platformIO
To use all Flash and PSRAM from the ESP32-S3-devkitc-1 with 16MB flash and 8MB PSRAM, copy the file `esp32-s3-devkitc-1-n16r8v.json` (within the intruments folder) to `C:\Users\<user>\.platformio\platforms\espressif32\boards\`.

Otherwise use `board = esp32-s3-devkitc-1`, but in this case only 8MB flash and no PSRAM is used.

## Flashing the ESP32
Download [MobiDude](https://github.com/elral/MobiDude/raw/main/build-win/MobiDude-1.0.5.zip) and extract the content to e.g. your Desktop.
Start `MobiDude.exe`, choose your your bin file, ESP32 as board, COM port, and press `Upload`
<img src="MobiDude.jpg" width="356"/>
Caution! Your bin file must contain `merged`
