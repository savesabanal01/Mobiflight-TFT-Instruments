# Mobiflight-TFT-Instruments

To use all Flash and PSRAM from the ESP32-S3-devkitc-1 with 16MB flash and 8MB PSRAM, copy the file `esp32-s3-devkitc-1-n16r8v.json` (within the intruments folder) to `C:\Users\<user>\.platformio\platforms\espressif32\boards\`.

Otherwise use `board = esp32-s3-devkitc-1`, but in this case only 8MB flash and no PSRAM is used.
