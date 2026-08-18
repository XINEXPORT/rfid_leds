/*
 * Title    :   RFID Controlled Mood Lighting
 * Author   :   Christine Hoang
 * Date     :   8/9/2026
 *
 * Purpose  :   This is a project to create an RFID Controlled Mood Lighting
                The intention of this project was to use RFID to turn on and off
                an LED light strip. This is ideal when you need to communicate
                to your loved ones or your coworkers when you are physically
                present in a room or when you don't want to be disturbed.

                It works by using RFID NFC UID tags that turn on and off an LED
                strip light. Each tag is programmed to a color scheme that
                corresponds to a mood. These tags could be affixed to objects,
                flat surfaces, or potentially your body!

                This method of turning on and off a light inspires playfulness to
                engage with objects in an environment without relying on pragmatic
                tools such as a light switch, a remote control, or a mobile device.
                But rather leveraging physical objects that transform the mood of
                a space.

                It also provides a way to customize your own mood lighting
                without relying on stock lighting schemes that most smart light
                applications provide. This is done by creating custom lighting
                code in C++ using the Arduino IDE which is then mapped to an RFID tag.

                MATERIALS USED:
                Arduino Minima R4
                Female to Male Jumper Wires
                Inland RC522 RFID Module
                RFID NFC Tags
                Female JST Connector
                SEZO 300 count LED Strip with JST Connectors
                USB-C Data Transfer Cable
                5v power supply
*/

/*
Project Imports
Description:    The project uses FastLED, SPI, MFRC522 and a tags_config.h file.
                To get the header files for FastLED, SPI, and MFRC522,
                install them from the Arduino IDE Managed Libraries.

                The tags_config.h file is where all the tag UIDs will be stored.
                There is a tags_config_example.h in the project files that you
                can reference so that you can create your own local file.
                Be sure to include a .gitignore file in the project that
                excludes tags_config.h from the commit history.
*/
#include <FastLED.h>
#include <SPI.h>
#include <MFRC522.h>
#include "tags_config.h"

/*
LED Variables
Description:    LED_PIN is referencing pin 6 on the Arduino Uno.
                NUM_LEDS is the total count of all LEDS on the LED strip.
                BRIGHTNESS sets the global LED brightness with a range from 0-255.
                LED_TYPE is the LED chipset driving the strip (WS2812B).
                COLOR_ORDER is the color channel order the strip expects (GRB).
*/
#define LED_PIN     6
#define NUM_LEDS    300
#define BRIGHTNESS  200
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

/*
RFID Settings
Description:    RST_PIN is pin 9 on the Arduino Uno that enables MFRC522 communication.
                SS_PIN is pin 10 on the Arduino Uno that enables SPI communication.
                rfid(SS_PIN, RST_PIN) creates the RFID reader object based on the
                pin variables.
*/
#define RST_PIN     9
#define SS_PIN      10

MFRC522 rfid(SS_PIN, RST_PIN);

/*
Light Mode Variables
Description:    leds[NUM_LEDS] is the FastLED pixel array, sized by NUM_LEDS.
                Mode enumerates the color schemes; currentMode tracks the
                active one and starts in OFF.
                Interval values control the refresh rate of each color
                scheme's animation, in milliseconds.
                Update values track when each color scheme last refreshed,
                and are initialized to 0.
*/
CRGB leds[NUM_LEDS];

enum Mode { OFF, RAINBOW, HALLOWEEN, OCEAN, SUNRISE };
Mode currentMode = OFF;

// OffSet values are initialized to 0 because the FastLED hue values range from 0-255.
uint8_t hueOffset = 0;
uint8_t halloweenOffset = 0;
uint8_t oceanOffset = 0;
uint8_t sunriseOffset = 0;


// Lower interval values are faster and higher interval values are slower.
const unsigned long RAINBOW_INTERVAL   = 20;
const unsigned long HALLOWEEN_INTERVAL = 200;
const unsigned long OCEAN_INTERVAL     = 10;
const unsigned long SUNRISE_INTERVAL   = 20;

unsigned long lastRainbowUpdate   = 0;
unsigned long lastHalloweenUpdate = 0;
unsigned long lastOceanUpdate     = 0;
unsigned long lastSunriseUpdate   = 0;

/*
Setup
Description: Opens Serial for status logging,
             registers the LED strip with FastLED and clears it,
             then starts SPI and initializes the
             RFID reader so it's ready to scan for tags.
*/
void setup() {
  Serial.begin(9600);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  SPI.begin();
  rfid.PCD_Init();
  Serial.println("Scanning for mood.");
}

/*
loop
Description: Runs continuously after setup. Checks for a newly scanned
             RFID tag, then runs the update function for whichever
             mode is currently active.
*/
void loop() {
  checkForCard();
  startScan();
}

/*
startScan
Description: Runs the update function for the currently active mode.
*/
void startScan() {
  if (currentMode == RAINBOW) {
    updateRainbow();
  } else if (currentMode == HALLOWEEN) {
    updateHalloween();
  } else if (currentMode == OCEAN) {
    updateOcean();
  } else if (currentMode == SUNRISE){
    updateSunrise();
  } else {
    updateOff();
  }
}

/*
updateRainbow
Description: Checks how much time has passed since the last frame and,
             once RAINBOW_INTERVAL has elapsed, redraws the strip.
             fill_rainbow spreads a full hue cycle evenly across
             NUM_LEDS starting at hueOffset = 0, so the whole strip shows
             a rainbow gradient. hueOffset is incremented each frame
             so the gradient appears to scroll along the strip, and
             FastLED.show() pushes the updated colors to the LEDs.
*/
void updateRainbow() {
  unsigned long now = millis();
  if (now - lastRainbowUpdate >= RAINBOW_INTERVAL) {
    lastRainbowUpdate = now;
    fill_rainbow(leds, NUM_LEDS, hueOffset, 150 / NUM_LEDS);
    hueOffset++;
    FastLED.show();
  }
}

/*
updateHalloween
Description: Checks how much time has passed since the last frame and,
             once HALLOWEEN_INTERVAL has elapsed, redraws the strip.
             drawHalloween splits the strip into blocks that alternate
             between orange and purple based on halloweenOffset.
             halloweenOffset is incremented each frame so the blocks
             appear to shift along the strip, and FastLED.show()
             pushes the updated colors to the LEDs.
*/
void updateHalloween() {
  unsigned long now = millis();
  if (now - lastHalloweenUpdate >= HALLOWEEN_INTERVAL) {
    lastHalloweenUpdate = now;
    drawHalloween();
    halloweenOffset++;
    FastLED.show();
  }
}

/*
updateOcean
Description: Checks how much time has passed since the last frame and,
             once OCEAN_INTERVAL has elapsed, redraws the strip.
             drawOcean paints a shifting hue gradient across the LEDs
             based on oceanOffset. oceanOffset is incremented each
             frame so the gradient appears to ripple along the strip,
             and FastLED.show() pushes the updated colors to the LEDs.
*/
void updateOcean() {
  unsigned long now = millis();
  if (now - lastOceanUpdate >= OCEAN_INTERVAL) {
    lastOceanUpdate = now;
    drawOcean();
    oceanOffset++;
    FastLED.show();
  }
}

/*
updateSunrise
Description: Checks how much time has passed since the last frame and,
             once SUNRISE_INTERVAL has elapsed, redraws the strip.
             drawSunrise paints a shifting hue gradient across the LEDs
             based on sunriseOffset. sunriseOffset is incremented each
             frame so the gradient appears to drift along the strip,
             and FastLED.show() pushes the updated colors to the LEDs.
*/
void updateSunrise() {
  unsigned long now = millis();
  if (now - lastSunriseUpdate >= SUNRISE_INTERVAL) {
    lastSunriseUpdate = now;
    drawSunrise();
    sunriseOffset++;
    FastLED.show();
  }
}

/*
updateOff
Description: Clears the strip to black.
*/
void updateOff() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

/*
drawHalloween
Description: Groups the strip into blocks of 2 LEDs and alternates
             each block between orange and purple. Adding
             halloweenOffset to the pixel index before dividing
             shifts the blocks over time, animating the pattern
             as it scrolls along the strip.
*/
void drawHalloween() {
  CRGB orange = CRGB(180, 60, 0);
  CRGB purple = CRGB(80, 0, 130);

  for (int i = 0; i < NUM_LEDS; i++) {
    if (((i + halloweenOffset) / 2) % 2 == 0) {
      leds[i] = orange;
    } else {
      leds[i] = purple;
    }
  }
}

/*
drawOcean
Description: Maps each LED's position along the strip to a point on
             a sine wave (sin8), then uses that wave to blend the hue
             between HUE_START and HUE_START + HUE_SPAN. CHSV makes
             it easy to sweep across a narrow hue band.
             Adding oceanOffset to the wave each frame shifts it over
             time, giving the gradient effect.
*/
void drawOcean() {
  const uint8_t HUE_START = 130;
  const uint8_t HUE_SPAN  = 35;

  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t position = (i * 255) / NUM_LEDS;
    uint8_t wave = sin8(position + oceanOffset);
    uint8_t hue = HUE_START + ((uint16_t)wave * HUE_SPAN) / 255;
    leds[i] = CHSV(hue, 150, 210);
  }
}

/*
drawSunrise
Description: Blends between light red, orange, and yellow using a
             palette, with each color defined as a CHSV value and
             converted to CRGB. Each LED's position plus sunriseOffset
             picks a point along the palette, and incrementing sunriseOffset
             each frame scrolls that point along the strip.
*/
void drawSunrise() {
  CHSV lightRed = CHSV(2, 210, 200);
  CHSV orange   = CHSV(20, 240, 220);
  CHSV yellow   = CHSV(0, 200, 200);

  CRGB lightRedRGB = lightRed;
  CRGB orangeRGB   = orange;
  CRGB yellowRGB   = yellow;
  CRGBPalette16 palette(lightRedRGB, orangeRGB, yellowRGB);

  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t position = (i * 255) / NUM_LEDS;
    leds[i] = ColorFromPalette(palette, position + sunriseOffset, 150, LINEARBLEND);
  }
}

/*
checkForCard
Description: Polls the RFID reader for a new tag. If none is present,
             returns immediately. Otherwise matches the tag's UID
             against the known tags and toggles the corresponding
             mode, or logs an unrecognized tag. Then halts the tag
             and stops the crypto session so the reader is ready for
             the next scan, with a short delay to avoid re-reading
             the same tap.
*/
void checkForCard() {
  if (not rfid.PICC_IsNewCardPresent() or not rfid.PICC_ReadCardSerial()) {
    return;
  }

  if (matchesUID(tag1UID)) {
    toggleRainbowMode();
  } else if (matchesUID(tag2UID)) {
    toggleHalloweenMode();
  } else if (matchesUID(tag3UID)) {
    toggleOceanMode();
  } else if (matchesUID(tag4UID)) {
    toggleSunriseMode();
  } else {
    Serial.print("Unrecognized tag. UID:");
    printUID();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(500);
}

/*
matchesUID
Description: Compares the UID of the most recently scanned tag
             against a known UID. Returns false right away if the
             sizes differ, otherwise compares byte by byte and
             returns false on the first mismatch. Only returns true
             if every byte matches.
*/
bool matchesUID(byte *knownUID) {
  if (rfid.uid.size != UID_LENGTH) return false;
  for (byte i = 0; i < UID_LENGTH; i++) {
    if (rfid.uid.uidByte[i] != knownUID[i]) return false;
  }
  return true;
}

/*
printUID
Description: Prints the UID of the most recently scanned tag to
             Serial in space-separated two-digit hex format.
*/
void printUID() {
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(' ');
    if (rfid.uid.uidByte[i] < 0x10) Serial.print('0');
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();
}

/*
toggleRainbowMode
Description: Toggles between RAINBOW and OFF when tag 1 is scanned.
*/
void toggleRainbowMode() {
  currentMode = (currentMode == RAINBOW) ? OFF : RAINBOW;
  Serial.print("Tag 1 scanned. Mode: ");
  Serial.println(currentMode == RAINBOW ? "RAINBOW" : "OFF");
}

/*
toggleHalloweenMode
Description: Toggles between HALLOWEEN and OFF when tag 2 is scanned.
*/
void toggleHalloweenMode() {
  currentMode = (currentMode == HALLOWEEN) ? OFF : HALLOWEEN;
  Serial.print("Tag 2 scanned. Mode: ");
  Serial.println(currentMode == HALLOWEEN ? "HALLOWEEN" : "OFF");
}

/*
toggleOceanMode
Description: Toggles between OCEAN and OFF when tag 3 is scanned.
*/
void toggleOceanMode() {
  currentMode = (currentMode == OCEAN) ? OFF : OCEAN;
  Serial.print("Tag 3 scanned. Mode: ");
  Serial.println(currentMode == OCEAN ? "OCEAN" : "OFF");
}

/*
toggleSunriseMode
Description: Toggles between SUNRISE and OFF when tag 4 is scanned.
*/
void toggleSunriseMode() {
  currentMode = (currentMode == SUNRISE) ? OFF : SUNRISE;
  Serial.print("Tag 4 scanned. Mode: ");
  Serial.println(currentMode == SUNRISE ? "SUNRISE" : "OFF");
}

