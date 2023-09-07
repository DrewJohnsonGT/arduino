// NeoPixel test program showing use of the WHITE channel for RGBW
// pixels only (won't look correct on regular RGB NeoPixel strips).

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>  // Required for 16 MHz Adafruit Trinket
#endif

#include "PinDefinitionsAndMore.h"  // Define macros for input and output pin etc.
#include <IRremote.hpp>

#include <Arduino.h>

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN 6

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 16

/*
 * Specify which protocol(s) should be used for decoding.
 * If no protocol is defined, all protocols (except Bang&Olufsen) are active.
 * This must be done before the #include <IRremote.hpp>
 */

#define DECODE_NEC

enum LIGHT_STATE {
  UP,
  ON,
  DOWN,
  BLAST,
  RAINBOW,
  FLASH_RED,
  BRIGHTNESS_UP,
  BRIGHTNESS_DOWN,
  SET_RED,
  SET_BLUE,
  SET_GREEN,
  SET_WHITE,
  RESET,
  CHASE,
  SPIN_BLAST
};

enum COLOR {
  WHITE,
  RED,
  GREEN,
  BLUE,
  OFF_WHITE
};

enum LIGHT_STATE lightState;

const int BRIGHTNESS_STEP = 25;
const int MAX_BRIGHTNESS = 255;
const int MIN_BRIGHTNESS = 0;

int BRIGHTNESS = 150;
int R = 0;
int G = 0;
int B = 0;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.
  Serial.begin(9600);

  strip.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();   // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);

  // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
}

void loop() {
  /*
     * Check if received data is available and if yes, try to decode it.
     * Decoded result is in the IrReceiver.decodedIRData structure.
     *
     * E.g. command is in IrReceiver.decodedIRData.command
     * address is in command is in IrReceiver.decodedIRData.address
     * and up to 32 bit raw data in IrReceiver.decodedIRData.decodedRawData
     */
  if (IrReceiver.decode()) {
    IrReceiver.resume();

    // CONTROLS
    const int command = IrReceiver.decodedIRData.command;
    Serial.println("--------------------------------------------------");
    Serial.print("IR command received: ");
    Serial.println(command);

    const int newLightState = getLightStateFromCommand(command);
    handleLightStateChange(newLightState);
  }
}

void handleLightStateChange(LIGHT_STATE newState) {
  Serial.print("Current light state: ");
  Serial.println(lightState);
  Serial.print("New light state: ");
  Serial.println(newState);

  if (newState == BRIGHTNESS_UP) {
    if (BRIGHTNESS <= MAX_BRIGHTNESS - BRIGHTNESS_STEP) {
      BRIGHTNESS += BRIGHTNESS_STEP;
      flashColor();
    }
  }
  if (newState == BRIGHTNESS_DOWN) {
    if (BRIGHTNESS >= BRIGHTNESS_STEP) {
      BRIGHTNESS -= BRIGHTNESS_STEP;
      flashColor();
    }
  }
  if (newState == FLASH_RED) {
    flashRed();
  }
  if (newState == ON) {
    strip.fill(strip.Color(R, G, B, BRIGHTNESS));
    strip.show();
  }
  if (newState == UP && lightState != UP) {
    colorUp(10);
  }
  if (newState == DOWN && lightState != DOWN) {
    colorDown(10);
  }
  if (newState == BLAST && lightState != BLAST) {
    blast();
  }
  if (newState == RAINBOW) {
    rainbow(5);
  }
  if (newState == SET_WHITE) {
    setColor(WHITE);
    flashColor();
  }
  if (newState == SET_RED) {
    setColor(RED);
    flashColor();
  }
  if (newState == SET_BLUE) {
    setColor(BLUE);
    flashColor();
  }
  if (newState == SET_GREEN) {
    setColor(GREEN);
    flashColor();
  }
  if (newState == CHASE) {
    theaterChaseRainbow(25);
  }
  if (newState == SPIN_BLAST) {
    theaterChase(strip.Color(127, 127, 127), 25);
  }

  lightState = newState;
}

LIGHT_STATE getLightStateFromCommand(int command) {
  if (command == 64) {
    return ON;
  }
  if (command == 67) {
    return UP;
  }
  if (command == 68) {
    return DOWN;
  }
  if (command == 9) {
    return BRIGHTNESS_UP;
  }
  if (command == 7) {
    return BRIGHTNESS_DOWN;
  }
  if (command == 69) {
    return BLAST;
  }
  if (command == 22) {
    return SET_WHITE;
  }
  if (command == 12) {
    return SET_RED;
  }
  if (command == 24) {
    return SET_GREEN;
  }
  if (command == 94) {
    return SET_BLUE;
  }
  if (command == 74) {
    return RAINBOW;
  }
  if (command == 71) {
    return RESET;
  }
  if (command == 82) {
    return CHASE;
  }
  if (command == 66) {
    return SPIN_BLAST;
  }
  return FLASH_RED;
  // 70: // VOL+
}
// Color
void setColor(COLOR color) {
  switch (color) {
    case WHITE:
      R = 0;
      G = 0;
      B = 0;
      return;
    case RED:
      R = 255;
      G = 0;
      B = 0;
      return;
    case GREEN:
      R = 0;
      G = 255;
      B = 0;
      return;
    case BLUE:
      R = 0;
      G = 0;
      B = 255;
      return;
    case OFF_WHITE:
      R = 248;
      G = 240;
      B = 227;
      return;
    default:
      R = 0;
      G = 0;
      B = 0;
      return;
  }
}

// Light functions
void flashColor() {
  for (int j = 0; j < 3; j++) {
    strip.fill(strip.Color(R, G, B, BRIGHTNESS));
    strip.show();
    delay(1);
    strip.clear();
    delay(1);
    strip.show();
  }
}
void flashRed() {
  for (int j = 0; j < 3; j++) {
    strip.fill(strip.Color(255, 0, 0, BRIGHTNESS));
    strip.show();
    delay(1);
    strip.clear();
    delay(1);
    strip.show();
  }
}

void blast() {
  int prevColor[] = {R, G, B};
  int b = MAX_BRIGHTNESS - (6 * BRIGHTNESS_STEP);
  setColor(OFF_WHITE);
  for (int j = 0; j < b; j++) {
    strip.fill(strip.Color(R, G, B, strip.gamma8(j)));
    strip.show();
    delay(5);
  }
  for (int j = b; j < MAX_BRIGHTNESS; j++) {
    strip.fill(strip.Color(R, G, B, strip.gamma8(j)));
    strip.show();
    delay(2);
  }
  delay(750);
  for (int j = MAX_BRIGHTNESS; j >= 0; j--) {  // Ramp down from 255 to 0
    strip.fill(strip.Color(R, G, B, strip.gamma8(j)));
    strip.show();
    delay(4);
  }
  R = prevColor[0];
  G = prevColor[1];
  B = prevColor[2];
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void colorUp(uint8_t wait) {
  for (int j = 0; j < BRIGHTNESS; j++) {
    for(uint16_t i=0; i<16; i++) {
      strip.setPixelColor(i, strip.Color(R, G, B, strip.gamma8(j)));
    }
    strip.show();
    delay(wait);
  }
}

void colorDown(uint8_t wait) {
  for (int j = BRIGHTNESS; j >= 0; j--) {
    for(uint16_t i=0; i<16; i++) {
      strip.setPixelColor(i, strip.Color(R, G, B, strip.gamma8(j)));
    }
    strip.show();
    delay(wait);
  }
}

void rainbow(int wait) {
  for (long firstPixelHue = 0; firstPixelHue < 3 * 65536; firstPixelHue += 256) {
    for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    delay(wait);
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}