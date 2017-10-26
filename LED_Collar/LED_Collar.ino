#include <FastLED.h>

//#define F_CPU 8000000UL

#if F_CPU == 8000000UL
__attribute__ ( ( section ( ".ramfunc" ) ) ) void delay_usec ( uint32_t n )
{
  __asm (
    "mydelay: \n"
    " sub  r0, r0, #1 \n"  // 1 cycle
    " nop             \n"  // 1 cycle
    " nop             \n"  // 1 cycle
    " nop             \n"  // 1 cycle
    " nop             \n"  // 1 cycle
    " nop             \n"  // 1 cycle
    " bne  mydelay    \n"  // 2 if taken, 1 otherwise
  );
}
#elif F_CPU == 48000000UL
__attribute__ ( ( section ( ".ramfunc" ) ) ) void delay_usec ( uint32_t n )
{
  __asm (
    "mydelay: \n"
    " mov  r1, #15    \n"  // 1 cycle
    "mydelay1: \n"
    " sub  r1, r1, #1 \n"  // 1 cycle
    " bne  mydelay1    \n" // 2 if taken, 1 otherwise
    " sub  r0, r0, #1 \n"  // 1 cycle
    " bne  mydelay    \n"  // 2 if taken, 1 otherwise
  );
}
#else
#error F_CPU is not defined
#endif

bool power_enable;

bool light_enable;
uint8_t brightness;
const uint8_t MAX_BRIGHTNESS = 64;
const uint8_t MIN_BRIGHTNESS = 0;

//LED Ring
#define LED_RING_COUNT 124

CRGB leds[LED_RING_COUNT];

#define POT_INPUT_PIN A0
#define SW_A_INPUT_PIN 18
#define SW_A_LED_PIN 10
#define SW_B_INPUT_PIN 15
#define SW_B_LED_PIN 11
#define SW_C_INPUT_PIN 17
#define SW_C_LED_PIN 12
#define SW_D_INPUT_PIN 16
#define SW_D_LED_PIN 13
#define SW_E_LED_PIN 5
#define SW_E_INPUT_PIN 9
#define SW_F_INPUT_PIN 19
#define SW_F_LED_PIN 6
#define SW_POWER_PIN 20
#define SW_TOGGLE_PIN 21
#define LED_PIN_COUNT 6

typedef enum {
    FADE_IN = 0,
    HOLD,
    FADE_OUT
  } led_state;

typedef struct {
  uint32_t timer;
  uint8_t value;
  uint8_t pin;
  led_state state;
} LED;

class FrontLEDs {
  public:
    FrontLEDs() {
      pinMode(SW_A_LED_PIN, OUTPUT);
      led[0].pin = SW_A_LED_PIN;
      pinMode(SW_B_LED_PIN, OUTPUT);
      led[1].pin = SW_B_LED_PIN;
      pinMode(SW_C_LED_PIN, OUTPUT);
      led[2].pin = SW_C_LED_PIN;
      pinMode(SW_D_LED_PIN, OUTPUT);
      led[3].pin = SW_D_LED_PIN;
      pinMode(SW_E_LED_PIN, OUTPUT);
      led[4].pin = SW_E_LED_PIN;
      pinMode(SW_F_LED_PIN, OUTPUT);
      led[5].pin = SW_F_LED_PIN;
      
      clearLEDS();
    }

    void clearLEDS() {
      for(uint8_t i = 0; i < LED_COUNT; ++i) {
        led[i].value = 0;
        led[i].state = HOLD;
        led[index].timer = millis();
      }
    };

    void FadeIn(uint8_t index, uint8_t value) {
      led[index].value = value;
      led[index].state = FADE_IN;
      led[index].timer = millis();
    }

    void FadeOut(uint8_t index, uint8_t value) {
      led[index].value = value;
      led[index].state = FADE_OUT;
      led[index].timer = millis();
    }

    void Draw() {
      for(uint8_t i = 0; i < LED_PIN_COUNT; ++i) {
        uint32_t delta_t = (millis() - led[i].timer);
        uint16_t value;
        if (delta_t > RATE) { 
          led[i].timer = millis();
          led[i].state = HOLD;
          continue;
        }
        switch(led[i].state) {
          case FADE_IN:
            value = (delta_t * led[i].value) / RATE;
            break;
          case FADE_OUT:
            value = ((RATE - delta_t) * led[i].value) / RATE;
            break;
          case HOLD:
          default: 
            value = led[i].value;
            break; 
        }
        analogWrite(led[i].pin, value);
      }
    }
    
  protected:
    LED led[LED_PIN_COUNT];
    static const uint8_t RATE = 500;
};

class Animation {
  public:
    virtual void Draw();
  protected:
    uint32_t timer = 0;
};

class Cylon : public Animation {
  public:
    Cylon(FrontLEDs *fled) {
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

 typedef struct {
      uint8_t start;
      uint8_t size;
      uint32_t timer;
      uint32_t life;
      uint8_t state;
} Arc;

class RandomArcs : public Animation {
  public:
    RandomArcs(FrontLEDs *fled) {
      fleds = fled;
      for(uint8_t i = 0; i < MAX_ARCS; ++i) {
        arcs[i] = newArc();
      }
    }

    void Draw() {
      //Start with NEWEST ARC (old arcs have preceedance)
      for(uint8_t i = 0; i < MAX_ARCS; ++i) {
        uint32_t advance_timer = (arcs[i].state != ArcStates::HOLD) ? FADE_TIME : arcs[i].life;
        uint32_t current_timer = millis() - arcs[i].timer;
        uint16_t value = 255;
        Serial.print("Arcs timing ");
        Serial.print(current_timer); Serial.print("/"); Serial.print(advance_timer);
        if (current_timer > advance_timer) {
           ++arcs[i].state;
           arcs[i].timer = current_timer = 0;
        }
        
        switch(arcs[i].state) {
          case ArcStates::FADE_IN:
            value *= current_timer;
            value /= FADE_TIME;
            break;
          case ArcStates::HOLD:
            break;
          case ArcStates::FADE_OUT:
            value *= FADE_TIME - current_timer;
            value /= FADE_TIME;
            break;
          case ArcStates::DEAD:
          default:
             value = 0;
             arcs[i] = newArc();
             break;
        }
        
        for(uint8_t j = 0; j < arcs[i].size; ++j) {
          uint8_t pixel_index = arcs[i].start + j;
          if (pixel_index >= LED_RING_COUNT) pixel_index -= LED_RING_COUNT;
          leds[pixel_index] = CRGB((uint8_t) value, 0,0);
        }

        for(uint8_t j = 0; j < LED_PIN_COUNT; ++j) {
          if (fleds->
        }
        
      }
      fleds->Draw();
      
      FastLED.show();
    }
    
  protected:
    static const uint8_t MAX_ARCS = 4;
    static const uint8_t MIN_ARC_SIZE = 4;
    static const uint32_t MAX_ARC_SIZE = LED_RING_COUNT/2;
    static const uint32_t MIN_ARC_LIFE = 1000;
    static const uint32_t MAX_ARC_LIFE = 5000;
    static const uint32_t FADE_TIME = 1000;
    enum ArcStates {
      FADE_IN = 0,
      HOLD,
      FADE_OUT,
      DEAD
    };
    Arc arcs[MAX_ARCS];
    FrontLEDs *fleds;

    Arc newArc() {
      Arc a;
      a.start = random(0, LED_RING_COUNT-1);
      a.size = random(MIN_ARC_SIZE, MAX_ARC_SIZE);
      a.timer = millis();
      a.life = random(MIN_ARC_LIFE, MAX_ARC_LIFE);
      a.state = FADE_IN;
      return a;
    }
};

class Random : public Animation {
  public:
    Random(FrontLEDs *fled) {
      fleds = fled;
      for(uint8_t i = 0; i < LED_RING_COUNT; ++i) {
        state[i] = false;
        timers[i] = millis();
      }
    }
  
    void Draw() {
      for(uint8_t i = 0; i < LED_RING_COUNT; ++i) {
        
        if (millis() > timers[i]) {
          state[i] = !state[i];
          timers[i] = random(MIN_PERIOD, MAX_PERIOD) + millis();
        }
        leds[i] = state[i] ? CRGB::Red : CRGB(0,0,0);
      }
      fleds->Draw();
      FastLED.show();
    }
    
  private:
    static const uint32_t MIN_PERIOD = 250;
    static const uint32_t MAX_PERIOD = 3000;
    uint32_t timers[LED_RING_COUNT];
    bool state[LED_RING_COUNT];
    FrontLEDs *fleds;
};

class Rotators : public Animation {
  public:
    Rotators(FrontLEDs *fleds) {
      offset = 0;
      timer = 0;
      uint8_t i = 0;
      uint8_t j = 0;
      uint8_t e = 4;
      while(i < LED_RING_COUNT) {
        for(j = 0; j < e; ++j) {
          if(j < e) {
            ring_buffer[i] = false;
          } else {
            ring_buffer[i] = true;
          }
          ++i;
        }
        e *= 2;
      }
    }

    void Draw() {
      if ((millis() - timer) > RATE) {
        timer = millis();
        FastLED.clear();
        uint8_t i = offset;
        uint8_t j = 0;
        do {
          if (ring_buffer[i]) {
            leds[j] = CRGB::Red;
          }
          ++i;
          ++j;
          if (i >= LED_RING_COUNT) {
            i = 0;
          }
        } while(j < LED_RING_COUNT);
        ++offset;
        FastLED.show();
      }
      fleds->Draw();
    }
  private:
    uint8_t offset;
    bool ring_buffer[LED_RING_COUNT];
    static const uint32_t RATE = 500;
    FrontLEDs *fleds;
};

Animation* current_animation = NULL;

typedef void(*input_callback)(bool state);

class Input {
  public:
    Input() {
      timer = millis();
      pinMode(SW_A_INPUT_PIN, INPUT_PULLUP);
      pinMode(SW_B_INPUT_PIN, INPUT_PULLUP);
      pinMode(SW_C_INPUT_PIN, INPUT_PULLUP);
      pinMode(SW_D_INPUT_PIN, INPUT_PULLUP);
      pinMode(SW_E_INPUT_PIN, INPUT_PULLUP);
      pinMode(SW_F_INPUT_PIN, INPUT_PULLUP);
      pinMode(SW_POWER_PIN, INPUT_PULLUP);
      pinMode(SW_TOGGLE_PIN, INPUT_PULLUP);

      for(uint8_t i = 0; i < DIGITAL_PIN_COUNT; ++i) {
        sw_states[i] = 0;
        callbacks[i] = NULL;
      }
      //Get initial states
      update();
    };

    void set_callback(uint8_t pin, input_callback callback) {
      callbacks[pin] = callback;
    };
  
    void edge_detect() {
      for(uint8_t i =0; i < DIGITAL_PIN_COUNT; ++i) {
        uint8_t value = digitalRead(pinmap[i]);
        Serial.print(i);
        Serial.print("::");
        Serial.print(value);
        Serial.print("|");
        if (value != sw_states[i]) {
          sw_states[i] = value;
          
          if(callbacks[i] != NULL)
            callbacks[i](value);
        }
      }
      Serial.println();
    };

    void update() {
      if ((millis() - timer) > POLLRATE) {
        timer = millis();
        edge_detect();
        uint16_t pot_value = analogRead(POT_INPUT_PIN);
        // Pot value is clamped to between 0 and 512
        if (pot_value > 511)
          pot_value = 511;
        if (pot_value <= 20) 
          pot_value = 0;
        brightness = MIN_BRIGHTNESS + ((MAX_BRIGHTNESS * pot_value) / 511);
      }
    };

  protected:
    static uint8_t const DIGITAL_PIN_COUNT = 8;
    static uint8_t const pinmap[DIGITAL_PIN_COUNT];
    static uint8_t const POLLRATE = 250;
    uint32_t timer;

    uint8_t sw_states[DIGITAL_PIN_COUNT];
    input_callback callbacks[DIGITAL_PIN_COUNT];
};

uint8_t const Input::pinmap[] = {
  SW_A_INPUT_PIN,
  SW_B_INPUT_PIN,
  SW_C_INPUT_PIN,
  SW_D_INPUT_PIN,
  SW_E_INPUT_PIN,
  SW_F_INPUT_PIN,
  SW_POWER_PIN,
  SW_TOGGLE_PIN
};

Input *input;
FrontLEDs *front_leds;

void selectAnimation(uint8_t animation) {
  delete current_animation;
  Serial.print("Setting anim "); Serial.println(animation); 
  switch(animation) {
    default:
    case 0:
      current_animation = new Cylon(front_leds);
      break;
    case 1:
      current_animation = new RandomArcs(front_leds);
      break;
    case 2:
      current_animation = new Random(front_leds);
      break;
    case 3:
      current_animation = new Rotators(front_leds);
      break;
    case 4:
    case 5:
      current_animation = new Cylon(front_leds);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  brightness = 32;
  light_enable = true;
  power_enable = true;
  input = new Input();
  front_leds = new FrontLEDs();
  input->set_callback(0, [](bool state) {if(!state){selectAnimation(0); front_leds->FadeIn(0, 255);}});
  input->set_callback(1, [](bool state) {if(!state)selectAnimation(1);});
  input->set_callback(2, [](bool state) {if(!state)selectAnimation(2);});
  input->set_callback(3, [](bool state) {if(!state)selectAnimation(3);});
  input->set_callback(4, [](bool state) {if(!state)selectAnimation(4);});
  input->set_callback(5, [](bool state) {if(!state)selectAnimation(5);});
  //Switches
  input->set_callback(6, [](bool state) {light_enable = state;});
  input->set_callback(7, [](bool state) {
    power_enable = !state;
    if (state){
      FastLED.clear();
      front_leds->clearLEDS();
      FastLED.show(); 
    }
  });
  FastLED.addLeds<APA102, BGR>(leds, LED_RING_COUNT);
  FastLED.setBrightness(brightness);
  selectAnimation(0);
  Serial.println("Complete");
}

void loop() {
  if (power_enable) {
    if (current_animation != NULL) {
      current_animation->Draw();
    }
    front_leds->Draw();
    input->update();
  }
}
