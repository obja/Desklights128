# _Desklights128_
=============

## ESP8266 Branch Information

### Installing ESP8266 with Boards Manager (via https://github.com/esp8266/Arduino )

Starting with 1.6.4, Arduino allows installation of third-party platform packages using Boards Manager. We have packages available for Windows, Mac OS, and Linux (32 and 64 bit).

- Install the current upstream Arduino IDE at the 1.8 level or later. The current version is at the [Arduino website](http://www.arduino.cc/en/main/software).
- Start Arduino and open Preferences window.
- Enter ```http://arduino.esp8266.com/stable/package_esp8266com_index.json``` into *Additional Board Manager URLs* field. You can add multiple URLs, separating them with commas.
- Open Boards Manager from Tools > Board menu and install *esp8266* platform (and don't forget to select your ESP8266 board from Tools > Board menu after installation).

### Teensyduino Installation
- Install Teensyduino via PJRC: https://www.pjrc.com/teensy/td_download.html


## Getting Started

- Set up Arduino for the Teensy (Tools > Board > Teensy 3.?)
- upload esp8266uploader.ino on to the Teensy (This is a copy of Teensy's example "USB_Serial > USBtoSerial" as of Teensyduino 1.37)

- Set up Arduino for the esp8266 (Tools > Board > Generic ESP8266 Module, Tools > Port > Teensy)
- upload esp8266.ino on to the esp8266 (This takes care of our WiFi connection and web requests)

- Set up Arduino for the Teensy again
- upload DeskLights128.ino (This receives serial data from the esp8266 and controls the LEDs)