#include <FastLED.h>

//LED Ring
#define LED_RING_COUNT 124

//#define DATA_PIN 10
//#define CLOCK_PIN 9

CRGB leds[LED_RING_COUNT];

void setup() {
  FastLED.addLeds<APA102, BGR>(leds, LED_RING_COUNT);
  FastLED.setBrightness(64);
}

void loop() {
  // put your main code here, to run repeatedly:
    // First slide the led in one direction
  for(int i = 0; i < LED_RING_COUNT; i++) {
    // Set the i'th led to red 
    leds[i] = CRGB::Red;
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    leds[i] = CRGB::Black;
    // Wait a little bit before we loop around and do it again
    delay(30);
  }

  // Now go in the other direction.  
  for(int i = LED_RING_COUNT-1; i >= 0; i--) {
    // Set the i'th led to red 
    leds[i] = CRGB::Red;
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    leds[i] = CRGB::Black;
    // Wait a little bit before we loop around and do it again
    delay(30);
  }
}
