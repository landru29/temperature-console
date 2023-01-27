# Temperature Console

## Components

* ESP32
* TFT touch screen 480x320
* temperature sensor DS18B20

## Prerequisite

You must install the following lib:

* https://github.com/Bodmer/TFT_eSPI
* https://github.com/PaulStoffregen/OneWire
* https://github.com/adafruit/Adafruit-GFX-Library
* https://github.com/adafruit/Adafruit_TouchScreen
* https://github.com/adafruit/TFTLCD-Library

## Configure TFT_eSPI

Edit file `Arduino/libraries/TFT_eSPI/User_Setup.h`:

1. comment line `#define ILI9341_DRIVER` => `// #define ILI9341_DRIVER`
2. Uncomment line `//#define ILI9488_DRIVER` => `#define ILI9488_DRIVER`

Edit file `Arduino/libraries/TFT_eSPI/User_Setup_Select.h`:

1. Uncomment line `//#include <User_Setups/Setup21_ILI9488.h>` => `#include <User_Setups/Setup21_ILI9488.h>`
2. Add line `#define TOUCH_CS 21`

## Install hardware on Arduino IDE

In case you haven't yet, you can add the ESP32 boards to your Arduino IDE by adding them to the Boards Manager: Open `File -> Preferences`, and paste the following URL in the `Additional Boards Manager URLs` field:

- `https://dl.espressif.com/dl/package_esp32_index.json`

Open the Boards Manager with `Tools -> Board: "xxx" -> Boards Manager`, look for `esp32` (probably the last one in the list), and click `Install`.

Finally, select the ESP32 board you have with `Tools -> Board: "xxx"` under the section `ESP32 Arduino` (I always have `ESP32 Dev Module` selected).