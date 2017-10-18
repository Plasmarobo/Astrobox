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
      forward = true;
    }
    
    void Draw() {
      if ((millis() - timer) > 30) {
        timer = millis();
        leds[i] = CRGB::Red;
        FastLED.show();
        leds[i] = CRGB::Black;
        if (forward) {
          ++i;
        } else {
          --i;
        }
        if (i >= LED_RING_COUNT) {
          --i;
          forward = false;
        } else if (i == 0) {
          forward = true;
        }
      }
    }
  private:
    uint8_t i;
    bool forward;
};

class Status : public Animation {
  public:
    Status() {
      i = 0;
      while(i < MAX_ARCS) {
        newArc();
      }
    }

    void Draw() {
      if ((millis() - timer) > GEN_RATE) {
        timer = millis();
        if (random(GEN_CHANCE) == 0) {
          newArc();
        }
      }
      
      fill_solid(leds, LED_RING_COUNT, CRGB::Red);
      
      for(uint8_t j = 0; j < MAX_ARCS; j += 2) {
        fill_solid(leds+arcs[j], arcs[j+1], CRGB::Blue);
      }
    }
  private:
    void newArc() {
      if (i >= MAX_ARCS) i = 0;
      uint8_t _size = random(6, LED_RING_COUNT / 4);
      arcs[i] = random(0, LED_RING_COUNT - _size);
      arcs[i+1] = arcs[i] + _size;
      i += 2;
    }
    
    static const uint8_t MAX_ARCS = 12;
    uint32_t timer;
    static const uint32_t GEN_RATE = 1000;
    static const uint8_t GEN_CHANCE = 4; //1/GEN_CHANCE
    uint8_t arcs[MAX_ARCS]; //ringbuffer
    uint8_t i;
};

class Rotators : public Animation {
  public:
    Rotators() {
      offset = 0;
      timer = 0;
      uint8_t i = 0;
      uint8_t j = 0;
      uint8_t e = 4;
      while(i < LED_RING_COUNT) {
        for(j = 0; j < 2*e; ++j) {
          if(j < e) {
            mask[i] = false;
          } else {
            mask[i] = true;
          }
          ++i;
        }
        e *= 2;
      }
    }

    void Draw() {
      if ((millis() - timer) > rate) {
        FastLED.clear();
        for(uint16_t i = 0, j = offset + 1; j != offset; ++j,++i) {
          if (j >= LED_RING_COUNT) j = 0;
          if (mask[j])
            leds[i] = CRGB::Red;
        }
        ++offset;
        FastLED.show();
      }
    }
  private:
    uint8_t offset;
    bool mask[LED_RING_COUNT];
    const uint16_t RATE = 250;
};

Animation* current_animation = NULL;

void selectAnimation(uint8_t animation) {
  delete current_animation;
  switch(animation) {
    case 1:
      current_animation = new Status();
      break;
    case 0:
    default:
      current_animation = new Cylon();
      break;
  }
}

void setup() {
  FastLED.addLeds<APA102, BGR>(leds, LED_RING_COUNT);
  FastLED.setBrightness(32);
  selectAnimation(1);
}

void loop() {
  if (current_animation != NULL) {
    current_animation->Draw();
  }
  pollInputs();
}
