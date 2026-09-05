// Part 1: control two body parts.

#include <FastLED.h>

#define LEDS_PER_STRIP 3
#define BODY_PART_1_PIN 2
#define BODY_PART_2_PIN 3

CRGB bodyPart1[LEDS_PER_STRIP];
CRGB bodyPart2[LEDS_PER_STRIP];

// Give every LED in one body part the same color.
void setBodyPart(CRGB leds[], CRGB color) {
  for (int i = 0; i < LEDS_PER_STRIP; i++) {
    leds[i] = color;
  }
}

void setup() {
  FastLED.addLeds<NEOPIXEL, BODY_PART_1_PIN>(bodyPart1, LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, BODY_PART_2_PIN>(bodyPart2, LEDS_PER_STRIP);

  FastLED.setBrightness(50);
  FastLED.clear(true);
}

void loop() {
  // First, light only body part 1.
  setBodyPart(bodyPart1, CRGB::Red);
  setBodyPart(bodyPart2, CRGB::Black);
  FastLED.show();
  delay(1000);

}

// Exercise:
// 1. Change the number of leds.
// 2. Put the 2 body parts on the same strip.
