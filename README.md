# RFID Controlled Mood Lighting
### Purpose
Tap an RFID/NFC tag to switch an LED strip between different color moods.

Each tag toggles a specific animated color scheme on and off:

| Tag    | Mode      | Color Mode Description                   |
| ------ | --------- | ---------------------------------------- |
| Tag 1  | Rainbow   | Scrolling full-spectrum gradient         |
| Tag 2  | Halloween | Alternating orange/purple blocks         |
| Tag 3  | Ocean     | Blue-green gradient                      |
| Tag 4  | Sunrise   | Red/orange/yellow gradient               |

Scanning a tag again turns its mode back off.

## Hardware

- Arduino Uno R4 Minima
- Inland RC522 RFID module
- RFID/NFC UID tags
- SEZO 300-count WS2812B LED strip w/ JST connectors
- Female JST connector
- Female-to-male jumper wires
- 5V power supply
- USB-C cable

### Wiring

| RC522 Pin | Arduino Pin |
| --------- | ----------- |
| SDA       | 10          |
| RST       | 9           |
| SPI       | 11-13       |

LED strip data line connects to pin 6.

## Software Setup

1. Install these libraries via the Arduino IDE Library Manager:
   - [FastLED](https://github.com/FastLED/FastLED)
   - [MFRC522](https://github.com/miguelbalboa/rfid)
   - SPI (bundled with the Arduino IDE)
2. Copy [tags_config_example.h](tags_config_example.h) to `tags_config.h`. Place
`tags_config.h` in the same folder as rfid_leds.ino
3. Scan each of your tags to retrieve the tag's UID and fill in `tag1UID` through `tag{n}UID` in `tags_config.h`.
4. Open [rfid_leds/rfid_leds.ino](rfid_leds/rfid_leds.ino) in the Arduino IDE, select your board, and upload.

## Configuration

Strip length, brightness, and pin assignments are defined at the top of [rfid_leds.ino](rfid_leds/rfid_leds.ino):

```cpp
#define LED_PIN     6
#define NUM_LEDS    300
#define BRIGHTNESS  200
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
```

Adjust these to match your own strip and wiring.

## Adding a New Mood

1. Add a UID variable for the new tag in `tags_config.h`.
2. Add a new value to the `Mode` enum in the `.ino` file.
3. Write a `drawYourMode()` function that paints `leds[]` for one frame, an `updateYourMode()` function that throttles it to an interval, and a `toggleYourMode()` function that flips `currentMode`.
4. Wire the new tag's UID into `checkForCard()`.
