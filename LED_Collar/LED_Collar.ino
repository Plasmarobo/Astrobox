#include <FastLED.h>

//INPUTS
#define POT_INPUT_PIN 14
#define SW_A_INPUT_PIN 13
#define SW_A_LED_PIN 12
#define SW_B_INPUT_PIN 15
#define SW_B_LED_PIN 11
#define SW_C_INPUT_PIN 16
#define SW_C_LED_PIN 10
#define SW_D_INPUT_PIN 17
#define SW_D_LED_PIN 9
#define SW_E_INPUT_PIN 18
#define SW_E_LED_PIN 6
#define SW_F_INPUT_PIN 19
#define SW_F_LED_PIN 5
#define POWER_SW_PIN 20
#define TOGGLE_SW_PIN 21

void pinConfig() {
  pinMode(SW_A_INPUT_PIN, INPUT_PULLUP);
  pinMode(SW_B_INPUT_PIN, INPUT_PULLUP);
  pinMode(SW_C_INPUT_PIN, INPUT_PULLUP);
  pinMode(SW_D_INPUT_PIN, INPUT_PULLUP);
  pinMode(SW_E_INPUT_PIN, INPUT_PULLUP);
  pinMode(SW_F_INPUT_PIN, INPUT_PULLUP);
  /*pinMode(SW_A_LED_PIN, OUTPUT);
  pinMode(SW_B_LED_PIN, OUTPUT);
  pinMode(SW_C_LED_PIN, OUTPUT);
  pinMode(SW_D_LED_PIN, OUTPUT);
  pinMode(SW_E_LED_PIN, OUTPUT);
  pinMode(SW_F_LED_PIN, OUTPUT);*/
}

void setLedBrightness(uint8_t led_pin, uint8_t brightness) {
  analogWrite(led_pin, brightness);
}

//LED Ring
#define LED_RING_COUNT 124

CRGB leds[LED_RING_COUNT];

class Animation {
  public:
    virtual void Draw();
  protected:
    uint32_t timer = 0;
};

class Cylon : public Animation {
  public:
    Cylon() {
      i = 0;
    }
    
    void Draw() {
      if ((millis() - timer) > 30) {
        uint8_t index = GetIndex();
        timer = millis();
        leds[index] = CRGB::Red;
        FastLED.show();
        leds[index] = CRGB::Black;
      }
    }
  private:
    uint8_t GetIndex() {
      if (i >= (2 * LED_RING_COUNT)) {
        i = 0;
      }
      if (i < LED_RING_COUNT) {
        return i++;
      } else {
        return LED_RING_COUNT - (i - LED_RING_COUNT);
      }
    }
    
    uint8_t i;
};

Animation* current_animation = NULL;

void selectAnimation(uint8_t animation) {
  delete current_animation;
  switch(animation) {
    case 0:
    default:
      current_animation = new Cylon();
      break;
  }
}

void setup() {
  FastLED.addLeds<APA102, BGR>(leds, LED_RING_COUNT);
  FastLED.setBrightness(32);
  selectAnimation(0);
}

void loop() {
  if (current_animation != NULL) {
    current_animation->Draw();
  }
}
