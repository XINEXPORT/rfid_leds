#include <SPI.h>

/*
 * Title    :   RFID Controlled Mood Lighting (Xmas Edition)
 * Author   :   Christine Hoang
 * Date     :   11/26/2026
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
#define NUM_LEDS    60
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

enum Mode { OFF, CLASSICTWINKLE, XMAS, EVERGREEN, RUDOLPH, CANDYCANE, HOLLY, TURBOSANTA, CLASSIC, DASHER, PRANCER, REINDEER, CYCLE };
Mode currentMode = OFF;

// Mode the CYCLE mode is currently showing. Kept as a byte so the cycle
// helper functions don't need the Mode type in their signatures.
byte cycleMode = CLASSICTWINKLE;

// OffSet values are initialized to 0 because the FastLED hue values range from 0-255.
uint8_t classicTwinkleOffset = 0;
uint8_t xmasOffset = 0;
uint8_t evergreenOffset = 0;
uint8_t rudolphOffset = 0;
uint8_t candyCaneOffset = 0;
uint8_t hollyOffset = 0;
uint8_t turboSantaOffset = 0;
uint8_t classicOffset = 0;
uint8_t dasherPosition = 0;
uint8_t prancerPosition = 0;
uint8_t reindeerPosition = 0;

// Mutables variables
unsigned long turboSantaInterval = 100;

//Boolean Flags
bool turboSantaAccelerating = true;
bool reindeerReverse = false;

// Lower interval values are faster and higher interval values are slower.
const unsigned long CLASSICTWINKLE_INTERVAL = 200;
const unsigned long XMAS_INTERVAL = 200;
const unsigned long EVERGREEN_INTERVAL     = 10;
const unsigned long RUDOLPH_INTERVAL   = 20;
const unsigned long CANDYCANE_INTERVAL = 80;
const unsigned long HOLLY_INTERVAL = 100;
const unsigned long TURBOSANTA_MIN_INTERVAL = 5;  
const unsigned long TURBOSANTA_MAX_INTERVAL = 100; 
const unsigned long TURBOSANTA_STEP = 2;  
const unsigned long CLASSIC_INTERVAL = 20;

// Classic bulb layout
const uint8_t CLASSIC_BULB_WIDTH = 2;
const uint8_t CLASSIC_BULB_GAP = 1;
const uint8_t CLASSIC_MIN_BRIGHTNESS = 90;

// Classic Twinkle layout: block width, and LEDs before its 5-color sequence repeats
const uint8_t CLASSICTWINKLE_BLOCK = 2;
const uint8_t CLASSICTWINKLE_PATTERN_LENGTH = CLASSICTWINKLE_BLOCK * 5;

const unsigned long DASHER_INTERVAL = 15;
const unsigned long PRANCER_INTERVAL = 15;
const unsigned long REINDEER_INTERVAL = 15;
const unsigned long CYCLE_INTERVAL = 10000;

unsigned long lastClassicTwinkleUpdate = 0;
unsigned long lastXmasUpdate = 0;
unsigned long lastEvergreenUpdate     = 0;
unsigned long lastRudolphUpdate   = 0;
unsigned long lastCandyCaneUpdate = 0;
unsigned long lastHollyUpdate = 0;
unsigned long lastTurboSantaUpdate = 0;
unsigned long lastClassicUpdate = 0;
unsigned long lastDasherUpdate = 0;
unsigned long lastPrancerUpdate = 0;
unsigned long lastReindeerUpdate = 0;
unsigned long lastCycleChange = 0;

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
nextCycleMode
Description: Returns the mode that follows the given one in the cycle.
             Steps through the enum in order, skipping OFF, and wraps
             from REINDEER (the last cycled mode) back to CLASSICTWINKLE.
*/
byte nextCycleMode(byte mode) {
  return (mode == REINDEER) ? CLASSICTWINKLE : mode + 1;
}

/*
activeMode
Description: Returns the mode whose animation should be drawn right
             now. Normally that is currentMode, but while CYCLE is
             active it is cycleMode, the mode the cycle is showing.
*/
byte activeMode() {
  return (currentMode == CYCLE) ? cycleMode : currentMode;
}

/*
advanceCycle
Description: While CYCLE is active, moves cycleMode on to the next
             mode once CYCLE_INTERVAL has elapsed. Does nothing in
             any other mode.
*/
void advanceCycle() {
  unsigned long now = millis();
  if (currentMode == CYCLE and now - lastCycleChange >= CYCLE_INTERVAL) {
    lastCycleChange = now;
    cycleMode = nextCycleMode(cycleMode);
    Serial.print("Cycle advanced to mode ");
    Serial.println(cycleMode);
  }
}

/*
startScan
Description: Advances the cycle if it is due, then runs the update
             function for whichever mode is active.
*/
void startScan() {
  advanceCycle();
  runMode(activeMode());
}

/*
runMode
Description: Runs the update function for the given mode.
*/
void runMode(byte mode) {
  if (mode == CLASSICTWINKLE) {
    updateClassicTwinkle();
  } else if (mode == XMAS) {
    updateXmas();
  } else if (mode == EVERGREEN) {
    updateEvergreen();
  } else if (mode == RUDOLPH) {
    updateRudolph();
  } else if (mode == CANDYCANE) {
    updateCandyCane();
  } else if (mode == HOLLY) {
    updateHolly();
  } else if (mode == TURBOSANTA) {
    updateTurboSanta();
  } else if (mode == CLASSIC) {
    updateClassic();
  } else if (mode == DASHER) {
    updateDasher();
  } else if (mode == PRANCER) {
    updatePrancer();
  } else if (mode == REINDEER) {
    updateReindeer();
  } else {
    updateOff();
  }
}

/*
updateClassicTwinkle
Description: Checks how much time has passed since the last frame and,
             once CLASSICTWINKLE_INTERVAL has elapsed, redraws the strip.
             drawClassicTwinkle splits the strip into blocks that step
             through the classic C9 colors based on classicTwinkleOffset.
             nextClassicTwinkleOffset moves them one LED each frame so the
             blocks appear to shift along the strip, like Xmas mode, and
             FastLED.show() pushes the updated colors to the LEDs.
*/
void updateClassicTwinkle() {
  unsigned long now = millis();
  if (now - lastClassicTwinkleUpdate >= CLASSICTWINKLE_INTERVAL) {
    lastClassicTwinkleUpdate = now;
    drawClassicTwinkle();
    classicTwinkleOffset = nextClassicTwinkleOffset(classicTwinkleOffset);
    FastLED.show();
  }
}

/*
nextClassicTwinkleOffset
Description: Returns the offset one LED further along. Wraps at
             CLASSICTWINKLE_PATTERN_LENGTH, the point where the color
             pattern repeats, so the motion stays smooth with no jump.
*/
uint8_t nextClassicTwinkleOffset(uint8_t offset) {
  return (offset + 1) % CLASSICTWINKLE_PATTERN_LENGTH;
}

/*
updateXmas
Description: Checks how much time has passed since the last frame and,
             once XMAS_INTERVAL has elapsed, redraws the strip.
             drawXmas splits the strip into blocks that alternate
             between red and green based on xmasOffset.
             xmasOffset is incremented each frame so the blocks
             appear to shift along the strip, and FastLED.show()
             pushes the updated colors to the LEDs.
*/
void updateXmas() {
  unsigned long now = millis();
  if (now - lastXmasUpdate >= XMAS_INTERVAL) {
    lastXmasUpdate = now;
    drawXmas();
    xmasOffset++;
    FastLED.show();
  }
}

/*
updateEvergreen
Description: Checks how much time has passed since the last frame and,
             once EVERGREEN_INTERVAL has elapsed, redraws the strip.
             drawEvergreen paints a shifting hue gradient across the LEDs
             based on evergreenOffset. evergreenOffset is incremented each
             frame so the gradient appears to ripple along the strip,
             and FastLED.show() pushes the updated colors to the LEDs.
*/
void updateEvergreen() {
  unsigned long now = millis();
  if (now - lastEvergreenUpdate >= EVERGREEN_INTERVAL) {
    lastEvergreenUpdate = now;
    drawEvergreen();
    evergreenOffset++;
    FastLED.show();
  }
}

/*
updateRudolph
Description: Checks how much time has passed since the last frame and,
             once RUDOLPH_INTERVAL has elapsed, redraws the strip.
             drawRudolph paints a shifting red gradient across the LEDs
             based on rudolphOffset. rudolphOffset is incremented each
             frame so the glow appears to drift along the strip,
             and FastLED.show() pushes the updated colors to the LEDs.
*/
void updateRudolph() {
  unsigned long now = millis();
  if (now - lastRudolphUpdate >= RUDOLPH_INTERVAL) {
    lastRudolphUpdate = now;
    drawRudolph();
    rudolphOffset++;
    FastLED.show();
  }
}

/*
updateCandyCane
Description: Checks how much time has passed since the last frame and,
             once CANDYCANE_INTERVAL has elapsed, redraws the strip.
             drawCandyCane paints alternating red and white stripes
             based on candyCaneOffset. candyCaneOffset is incremented
             each frame so the stripes appear to wind along the strip,
             and FastLED.show() pushes the updated colors to the LEDs.
*/
void updateCandyCane() {
  unsigned long now = millis();
  if (now - lastCandyCaneUpdate >= CANDYCANE_INTERVAL) {
    lastCandyCaneUpdate = now;
    drawCandyCane();
    candyCaneOffset++;
    FastLED.show();
  }
}

/*
updateHolly
Description: Checks how much time has passed since the last frame and,
             once HOLLY_INTERVAL has elapsed, redraws the strip.
             drawHolly paints a shifting red and green gradient across
             the LEDs based on hollyOffset. hollyOffset is incremented
             each frame so the gradient appears to drift along the
             strip, and FastLED.show() pushes the updated colors to
             the LEDs.
*/
void updateHolly() {
  unsigned long now = millis();
  if (now - lastHollyUpdate >= HOLLY_INTERVAL) {
    lastHollyUpdate = now;
    drawHolly();
    hollyOffset++;
    FastLED.show();
  }
}

/*
updateTurboSanta
Description: Redraws the chase once turboSantaInterval has elapsed, then
             speeds up or slows down via accelerateTurboSanta/decelerateTurboSanta,
             cycling endlessly between TURBOSANTA_MIN_INTERVAL and
             TURBOSANTA_MAX_INTERVAL.
*/
void updateTurboSanta() {
  unsigned long now = millis();
  if (now - lastTurboSantaUpdate >= turboSantaInterval) {
    lastTurboSantaUpdate = now;
    drawTurboSanta();
    turboSantaOffset = (turboSantaOffset + 1) % 12;
    FastLED.show();

    if (turboSantaAccelerating) {
		accelerateTurboSanta();
    } else {
		decelerateTurboSanta();
    }
  }
}

/*
updateClassic
Description: Checks how much time has passed since the last frame and,
             once CLASSIC_INTERVAL has elapsed, redraws the strip.
             drawClassic paints C9-style multicolor bulbs whose even and
             odd bulbs glow up and down in turns based on classicOffset.
             classicOffset is incremented each frame so the glow keeps
             moving, and FastLED.show() pushes the updated colors to
             the LEDs.
*/
void updateClassic() {
  unsigned long now = millis();
  if (now - lastClassicUpdate >= CLASSIC_INTERVAL) {
    lastClassicUpdate = now;
    drawClassic();
    classicOffset++;
    FastLED.show();
  }
}

/*
updateDasher
Description: Checks how much time has passed since the last frame and,
             once DASHER_INTERVAL has elapsed, redraws the strip.
             drawDasher paints a single green dash at dasherPosition.
             nextStrokePosition moves the dash one LED along the strip
             and restarts it at the beginning once it has fully left
             the far end, and FastLED.show() pushes the updated colors
             to the LEDs.
*/
void updateDasher() {
  unsigned long now = millis();
  if (now - lastDasherUpdate >= DASHER_INTERVAL) {
    lastDasherUpdate = now;
    drawDasher();
    dasherPosition = nextStrokePosition(dasherPosition);
    FastLED.show();
  }
}

/*
updatePrancer
Description: Checks how much time has passed since the last frame and,
             once PRANCER_INTERVAL has elapsed, redraws the strip.
             drawPrancer paints a single red dash at prancerPosition,
             running from the far end of the strip back toward the
             start. nextStrokePosition advances the dash and restarts
             it once it has fully left the strip, and FastLED.show()
             pushes the updated colors to the LEDs.
*/
void updatePrancer() {
  unsigned long now = millis();
  if (now - lastPrancerUpdate >= PRANCER_INTERVAL) {
    lastPrancerUpdate = now;
    drawPrancer();
    prancerPosition = nextStrokePosition(prancerPosition);
    FastLED.show();
  }
}

/*
updateReindeer
Description: Checks how much time has passed since the last frame and,
             once REINDEER_INTERVAL has elapsed, redraws the strip.
             drawReindeer paints whichever reindeer is running: Dasher's
             green stroke forward or Prancer's red stroke in reverse.
             nextStrokePosition advances the stroke, and when it wraps
             back to 0 the stroke has finished, so reindeerReverse flips
             and the other reindeer takes the next run. FastLED.show()
             pushes the updated colors to the LEDs.
*/
void updateReindeer() {
  unsigned long now = millis();
  if (now - lastReindeerUpdate >= REINDEER_INTERVAL) {
    lastReindeerUpdate = now;
    drawReindeer();
    reindeerPosition = nextStrokePosition(reindeerPosition);
    reindeerReverse = (reindeerPosition == 0) ? not reindeerReverse : reindeerReverse;
    FastLED.show();
  }
}

/*
nextStrokePosition
Description: Returns a stroke's position one step further along the
             strip. Runs past the last LED by STROKE_RUNOFF steps so the
             dash and its tail can fully leave the strip, then wraps
             back to 0 to start the next stroke. Shared by Dasher and
             Prancer.
*/
uint8_t nextStrokePosition(uint8_t position) {
  const uint8_t STROKE_RUNOFF = 20;
  return (position + 1) % (NUM_LEDS + STROKE_RUNOFF);
}

/*
accelerateTurboSanta
Description: Decreases turboSantaInterval toward TURBOSANTA_MIN_INTERVAL
             each call, speeding up the chase. Once the minimum is
             reached, the pattern will decelerate.
*/
void accelerateTurboSanta(){
  if (turboSantaInterval > TURBOSANTA_MIN_INTERVAL + TURBOSANTA_STEP) {
    turboSantaInterval -= TURBOSANTA_STEP;
  } else {
    turboSantaInterval = TURBOSANTA_MIN_INTERVAL;
    turboSantaAccelerating = false;
  }
}

/*
decelerateTurboSanta
Description: Increases turboSantaInterval toward TURBOSANTA_MAX_INTERVAL
             each call, slowing the chase down. Once the maximum is
             reached, the pattern will accelerate.
*/
void decelerateTurboSanta(){
  if (turboSantaInterval < TURBOSANTA_MAX_INTERVAL - TURBOSANTA_STEP) {
    turboSantaInterval += TURBOSANTA_STEP;
  } else {
    turboSantaInterval = TURBOSANTA_MAX_INTERVAL;
    turboSantaAccelerating = true;
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
drawXmas
Description: Groups the strip into blocks of 2 LEDs and alternates
             each block between red and green. Adding
             xmasOffset to the pixel index before dividing
             shifts the blocks over time, animating the pattern
             as it scrolls along the strip.
*/
void drawXmas() {
  CRGB red   = CRGB(180, 0, 0);
  CRGB green = CRGB(0, 130, 10);

  for (int i = 0; i < NUM_LEDS; i++) {
    if (((i + xmasOffset) / 2) % 2 == 0) {
      leds[i] = red;
    } else {
      leds[i] = green;
    }
  }
}

/*
drawEvergreen
Description: Maps each LED's position along the strip to a point on
             a sine wave (sin8), then uses that wave to blend the hue
             between HUE_START and HUE_START + HUE_SPAN, a narrow band
             of pine greens. The same wave also dims the LEDs slightly,
             so the darker, bluer greens read as shadowed needles and
             the brighter, yellower greens as lit ones. CHSV makes
             it easy to sweep across a narrow hue band.
             Adding evergreenOffset to the wave each frame shifts it over
             time, giving the gradient effect.
*/
void drawEvergreen() {
  const uint8_t HUE_START = 85;
  const uint8_t HUE_SPAN  = 25;
  const uint8_t VAL_MIN   = 90;
  const uint8_t VAL_SPAN  = 80;

  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t position = (i * 255) / NUM_LEDS;
    uint8_t wave = sin8(position + evergreenOffset);
    uint8_t hue = HUE_START + ((uint16_t)(255 - wave) * HUE_SPAN) / 255;
    uint8_t val = VAL_MIN + ((uint16_t)wave * VAL_SPAN) / 255;
    leds[i] = CHSV(hue, 240, val);
  }
}

/*
drawRudolph
Description: Blends between dark red, bright red, and a glowing
             red-orange using a palette, with each color defined as a
             CHSV value and converted to CRGB. The palette ends on dark
             red again so the gradient loops without a visible seam.
             Each LED's position plus rudolphOffset picks a point along
             the palette, and incrementing rudolphOffset each frame
             scrolls that point along the strip.
*/
void drawRudolph() {
  CHSV darkRed = CHSV(0, 255, 60);
  CHSV red     = CHSV(0, 255, 200);
  CHSV glow    = CHSV(4, 230, 255);

  CRGB darkRedRGB = darkRed;
  CRGB redRGB     = red;
  CRGB glowRGB    = glow;
  CRGBPalette16 palette(darkRedRGB, redRGB, glowRGB, darkRedRGB);

  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t position = (i * 255) / NUM_LEDS;
    leds[i] = ColorFromPalette(palette, position + rudolphOffset, 200, LINEARBLEND);
  }
}

/*
drawCandyCane
Description: Draws red and white stripes like a candy cane. Groups the
             strip into blocks of CANDYCANE_STRIPE LEDs and alternates
             each block between red and white. Adding candyCaneOffset
             to the pixel index before dividing shifts the stripes
             over time, so they appear to wind along the strip.
             The white is held below full intensity to keep the strip's
             current draw within what a 5v supply can handle.
*/
void drawCandyCane() {
  const uint8_t CANDYCANE_STRIPE = 4;
  CRGB red   = CRGB(180, 0, 0);
  CRGB white = CRGB(140, 140, 140);

  for (int i = 0; i < NUM_LEDS; i++) {
    if (((i + candyCaneOffset) / CANDYCANE_STRIPE) % 2 == 0) {
      leds[i] = red;
    } else {
      leds[i] = white;
    }
  }
}

/*
drawHolly
Description: Blends between holly green and berry red using a palette,
             with each color defined as a CHSV value and converted to
             CRGB. Each color fades through black before the next one,
             so red and green never mix into orange or yellow. The
             palette ends on black so the gradient loops without a
             visible seam. Each LED's position plus hollyOffset picks a
             point along the palette, and incrementing hollyOffset each
             frame scrolls that point along the strip.
*/
void drawHolly() {
  CHSV hollyGreen = CHSV(96, 255, 160);
  CHSV berryRed   = CHSV(0, 255, 180);

  CRGB hollyGreenRGB = hollyGreen;
  CRGB berryRedRGB   = berryRed;
  CRGB blackRGB      = CRGB::Black;
  CRGBPalette16 palette(hollyGreenRGB, blackRGB, berryRedRGB, blackRGB);

  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t position = (i * 255) / NUM_LEDS;
    leds[i] = ColorFromPalette(palette, position + hollyOffset, 200, LINEARBLEND);
  }
}

/*
drawTurboSanta
Description: Draws a marquee-style chase pattern - blocks of 3 LEDs
             that repeat red, off, green, off, based on turboSantaOffset.
             The dark blocks keep red and green apart so the chase reads
             cleanly at high speed. Shifting the block boundary by
             turboSantaOffset each frame makes the pattern appear to
             scroll along the strip.
*/
void drawTurboSanta() {
  CRGB red   = CHSV(0, 255, 150);
  CRGB green = CHSV(96, 255, 150);

  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t block = ((i + turboSantaOffset) / 3) % 4;
    if (block == 0) {
      leds[i] = red;
    } else if (block == 2) {
      leds[i] = green;
    } else {
      leds[i] = CRGB::Black;
    }
  }
}

/*
drawClassic
Description: Draws old-fashioned C9 string lights. The strip is split
             into bulbs CLASSIC_BULB_WIDTH LEDs wide with a dark
             CLASSIC_BULB_GAP between them. Each bulb gets the next
             color in the classic red, green, blue, amber, warm white
             sequence, and is dimmed by classicBrightness so the even
             and odd bulbs glow up and down in turns, like the "in
             waves" setting on a light-string controller.
*/
void drawClassic() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = isClassicBulb(i)
      ? dimmed(classicColor(classicBulbIndex(i)), classicBrightness(classicBulbIndex(i)))
      : CRGB(CRGB::Black);
  }
}

/*
drawClassicTwinkle
Description: Works like drawXmas, but with the classic C9 colors.
             Groups the strip into blocks of CLASSICTWINKLE_BLOCK LEDs
             and gives each block the next color in the red, green,
             blue, amber, warm white sequence. Adding classicTwinkleOffset
             to the pixel index before dividing shifts the blocks over
             time, animating the pattern as it scrolls along the strip.
*/
void drawClassicTwinkle() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = classicColor((i + classicTwinkleOffset) / CLASSICTWINKLE_BLOCK);
  }
}

/*
isClassicBulb
Description: Returns true if LED i is part of a bulb, false if it falls
             in the dark gap between bulbs.
*/
bool isClassicBulb(int i) {
  return (i % (CLASSIC_BULB_WIDTH + CLASSIC_BULB_GAP)) < CLASSIC_BULB_WIDTH;
}

/*
classicBulbIndex
Description: Returns which bulb LED i belongs to, counting from the
             start of the strip.
*/
uint8_t classicBulbIndex(int i) {
  return i / (CLASSIC_BULB_WIDTH + CLASSIC_BULB_GAP);
}

/*
classicColor
Description: Returns the color for the given bulb, stepping through the
             classic C9 sequence of red, green, blue, amber and warm white.
*/
CRGB classicColor(uint8_t bulb) {
  static const CRGB COLORS[] = {
    CRGB(200, 0, 0),     // red
    CRGB(0, 160, 0),     // green
    CRGB(0, 0, 200),     // blue
    CRGB(220, 90, 0),    // amber
    CRGB(200, 120, 40)   // warm white
  };
  return COLORS[bulb % 5];
}

/*
classicBrightness
Description: Returns the bulb's brightness for this frame. sin8 turns
             classicOffset into a smooth wave; odd bulbs are shifted
             half a wave (128) so they are bright while the even bulbs
             are dim. The wave is scaled into CLASSIC_MIN_BRIGHTNESS-255
             so bulbs dim but never go fully dark.
*/
uint8_t classicBrightness(uint8_t bulb) {
  return CLASSIC_MIN_BRIGHTNESS
    + scale8(sin8(classicOffset + (bulb % 2) * 128), 255 - CLASSIC_MIN_BRIGHTNESS);
}

/*
dimmed
Description: Returns a copy of color scaled down to the given brightness.
*/
CRGB dimmed(CRGB color, uint8_t brightness) {
  return color.nscale8_video(brightness);
}

/*
drawStroke
Description: Draws a single dash of the given color that streaks along
             the strip. fadeToBlackBy dims every LED a little each
             frame, so the LEDs the dash just passed leave a fading
             tail behind it. The dash is STROKE_LENGTH LEDs long, ending
             at position; any part of it off the strip is skipped.
             When reverse is true, positions are mirrored so the dash
             runs from the far end of the strip back to the start.
             Shared by Dasher and Prancer.
*/
void drawStroke(uint8_t position, CRGB color, bool reverse) {
  const uint8_t STROKE_LENGTH = 4;
  const uint8_t STROKE_FADE   = 60;

  fadeToBlackBy(leds, NUM_LEDS, STROKE_FADE);

  for (int i = position - STROKE_LENGTH + 1; i <= position; i++) {
    if (i >= 0 and i < NUM_LEDS) {
      leds[reverse ? NUM_LEDS - 1 - i : i] = color;
    }
  }
}

/*
drawDasher
Description: Draws Dasher's green stroke running from the start of the
             strip to the far end.
*/
void drawDasher() {
  drawStroke(dasherPosition, CHSV(96, 255, 200), false);
}

/*
drawPrancer
Description: Draws Prancer's red stroke running from the far end of the
             strip back to the start.
*/
void drawPrancer() {
  drawStroke(prancerPosition, CHSV(0, 255, 200), true);
}

/*
drawReindeer
Description: Draws the current reindeer's stroke: Dasher's green going
             forward when reindeerReverse is false, Prancer's red going
             in reverse when it is true.
*/
void drawReindeer() {
  drawStroke(reindeerPosition, reindeerColor(reindeerReverse), reindeerReverse);
}

/*
reindeerColor
Description: Returns Prancer's red for a reverse run and Dasher's green
             for a forward run.
*/
CRGB reindeerColor(bool reverse) {
  return reverse ? CRGB(CHSV(0, 255, 200)) : CRGB(CHSV(96, 255, 200));
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

  if (matchesUID(tag1UID)) {            //classic twinkle
    toggleClassicTwinkleMode();       
  } else if (matchesUID(tag2UID)) {     //xmas
    toggleXmasMode();
  } else if (matchesUID(tag3UID)) {     //evergreen
    toggleEvergreenMode();
  } else if (matchesUID(tag4UID)) {     //rudolph
    toggleRudolphMode();
  } else if (matchesUID(tag5UID)) {     //candycane
    toggleCandyCaneMode();
  } else if (matchesUID(tag6UID)) {     //holly
    toggleHollyMode();
  } else if(matchesUID(tag7UID)) {      //turbo santa
    toggleTurboSantaMode();
  } else if (matchesUID(tag8UID)) {     //dasher
    toggleDasherMode();
  } else if (matchesUID(tag9UID)) {     //prancer
    togglePrancerMode();
  } else if (matchesUID(tag10UID)) {    //reindeer
    toggleReindeerMode();
  } else if (matchesUID(tag11UID)) {    //cycle
    toggleCycleMode();
  } else if (matchesUID(tag12UID)) {    //classic
    toggleClassicMode();
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
toggleClassicTwinkleMode
Description: Toggles between CLASSICTWINKLE and OFF when tag 1 is scanned.
*/
void toggleClassicTwinkleMode() {
  currentMode = (currentMode == CLASSICTWINKLE) ? OFF : CLASSICTWINKLE;
  Serial.print("Tag 1 scanned. Mode: ");
  Serial.println(currentMode == CLASSICTWINKLE ? "CLASSIC TWINKLE" : "OFF");
}

/*
toggleXmasMode
Description: Toggles between XMAS and OFF when tag 2 is scanned.
*/
void toggleXmasMode() {
  currentMode = (currentMode == XMAS) ? OFF : XMAS;
  Serial.print("Tag 2 scanned. Mode: ");
  Serial.println(currentMode == XMAS ? "XMAS" : "OFF");
}

/*
toggleEvergreenMode
Description: Toggles between EVERGREEN and OFF when tag 3 is scanned.
*/
void toggleEvergreenMode() {
  currentMode = (currentMode == EVERGREEN) ? OFF : EVERGREEN;
  Serial.print("Tag 3 scanned. Mode: ");
  Serial.println(currentMode == EVERGREEN ? "EVERGREEN" : "OFF");
}

/*
toggleRudolphMode
Description: Toggles between RUDOLPH and OFF when tag 4 is scanned.
*/
void toggleRudolphMode() {
  currentMode = (currentMode == RUDOLPH) ? OFF : RUDOLPH;
  Serial.print("Tag 4 scanned. Mode: ");
  Serial.println(currentMode == RUDOLPH ? "RUDOLPH'S NOSE" : "OFF");
}

/*
toggleCandyCaneMode
Description: Toggles between CANDYCANE and OFF when tag 5 is scanned.
*/
void toggleCandyCaneMode() {
  currentMode = (currentMode == CANDYCANE) ? OFF : CANDYCANE;
  Serial.print("Tag 5 scanned. Mode: ");
  Serial.println(currentMode == CANDYCANE ? "CANDY CANE" : "OFF");
}

/*
toggleHollyMode
Description: Toggles between HOLLY and OFF when tag 6 is scanned.
*/
void toggleHollyMode() {
  currentMode = (currentMode == HOLLY) ? OFF : HOLLY;
  Serial.print("Tag 6 scanned. Mode: ");
  Serial.println(currentMode == HOLLY ? "HOLLY" : "OFF");
}

/*
toggleTurboSantaMode
Description: Toggles between TURBOSANTA and OFF when tag7 is scanned.
             Resets speed state to the slow starting point.
*/
void toggleTurboSantaMode() {
  currentMode = (currentMode == TURBOSANTA) ? OFF : TURBOSANTA;
  turboSantaInterval = TURBOSANTA_MAX_INTERVAL;
  turboSantaAccelerating = true;
  Serial.print("Tag 7 scanned. Mode: ");
  Serial.println(currentMode == TURBOSANTA ? "TURBO SANTA" : "OFF");
}

/*
toggleClassicMode
Description: Toggles between CLASSIC and OFF when tag 12 is scanned.
*/
void toggleClassicMode() {
  currentMode = (currentMode == CLASSIC) ? OFF : CLASSIC;
  Serial.print("Tag 12 scanned. Mode: ");
  Serial.println(currentMode == CLASSIC ? "CLASSIC" : "OFF");
}

/*
toggleDasherMode
Description: Toggles between DASHER and OFF when tag 8 is scanned.
             Clears the strip and restarts the dash at the beginning
             so every stroke starts clean.
*/
void toggleDasherMode() {
  currentMode = (currentMode == DASHER) ? OFF : DASHER;
  dasherPosition = 0;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  Serial.print("Tag 8 scanned. Mode: ");
  Serial.println(currentMode == DASHER ? "DASHER" : "OFF");
}

/*
togglePrancerMode
Description: Toggles between PRANCER and OFF when tag 9 is scanned.
             Clears the strip and restarts the dash at the far end
             so every stroke starts clean.
*/
void togglePrancerMode() {
  currentMode = (currentMode == PRANCER) ? OFF : PRANCER;
  prancerPosition = 0;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  Serial.print("Tag 9 scanned. Mode: ");
  Serial.println(currentMode == PRANCER ? "PRANCER" : "OFF");
}

/*
toggleReindeerMode
Description: Toggles between REINDEER and OFF when tag 10 is scanned.
             Clears the strip and restarts with Dasher's forward run
             so the relay always starts clean.
*/
void toggleReindeerMode() {
  currentMode = (currentMode == REINDEER) ? OFF : REINDEER;
  reindeerPosition = 0;
  reindeerReverse = false;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  Serial.print("Tag 10 scanned. Mode: ");
  Serial.println(currentMode == REINDEER ? "REINDEER" : "OFF");
}

/*
toggleCycleMode
Description: Toggles between CYCLE and OFF when tag 11 is scanned.
             Restarts the cycle from CLASSICTWINKLE and resets the timer so
             each mode gets its full CYCLE_INTERVAL.
*/
void toggleCycleMode() {
  currentMode = (currentMode == CYCLE) ? OFF : CYCLE;
  cycleMode = CLASSICTWINKLE;
  lastCycleChange = millis();
  Serial.print("Tag 11 scanned. Mode: ");
  Serial.println(currentMode == CYCLE ? "CYCLE" : "OFF");
}
