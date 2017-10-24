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

 typedef struct {
      uint8_t start;
      uint8_t size;
      uint32_t timer;
      uint32_t life;
      uint8_t state;
} Arc;

class RandomArcs : public Animation {
  public:
    RandomArcs() {
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
      }
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
    Random() {
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
      FastLED.show();
    }
    
  private:
    static const uint32_t MIN_PERIOD = 250;
    static const uint32_t MAX_PERIOD = 3000;
    uint32_t timers[LED_RING_COUNT];
    bool state[LED_RING_COUNT];
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
    }
  private:
    uint8_t offset;
    bool ring_buffer[LED_RING_COUNT];
    static const uint32_t RATE = 500;
};

Animation* current_animation = NULL;

typedef void(*io_callback)(bool io);

class IO {
  public:
    IO() {
      timer = millis();
      pinMode(SW_A_INPUT_PIN, INPUT_PULLUP);
      pinMode(SW_B_INPUT_PIN, INPUT_PULLUP);
      pinMode(SW_C_INPUT_PIN, INPUT_PULLUP);
      pinMode(SW_D_INPUT_PIN, INPUT_PULLUP);
      pinMode(SW_E_INPUT_PIN, INPUT_PULLUP);
      pinMode(SW_F_INPUT_PIN, INPUT_PULLUP);
      pinMode(SW_POWER_PIN, INPUT_PULLUP);
      pinMode(SW_TOGGLE_PIN, INPUT_PULLUP);
      
      pinMode(SW_A_LED_PIN, OUTPUT);
      pinMode(SW_B_LED_PIN, OUTPUT);
      pinMode(SW_C_LED_PIN, OUTPUT);
      pinMode(SW_D_LED_PIN, OUTPUT);
      pinMode(SW_E_LED_PIN, OUTPUT);
      pinMode(SW_F_LED_PIN, OUTPUT);
      clearLEDS();
      for(uint8_t i = 0; i < DIGITAL_PIN_COUNT; ++i) {
        sw_states[i] = 0;
        callbacks[i] = NULL;
      }
      //Get initial states
      update();
    };

    void set_callback(uint8_t pin, io_callback callback) {
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

    void clearLEDS() {
      analogWrite(SW_A_LED_PIN, 0);
      analogWrite(SW_B_LED_PIN, 0);
      analogWrite(SW_C_LED_PIN, 0);
      analogWrite(SW_D_LED_PIN, 0);
      analogWrite(SW_E_LED_PIN, 0);
      analogWrite(SW_F_LED_PIN, 0);
    };

  protected:
    static uint8_t const POT_INPUT_PIN = A0;
    static uint8_t const SW_A_INPUT_PIN = 18;
    static uint8_t const SW_A_LED_PIN = 10;
    static uint8_t const SW_B_INPUT_PIN = 15;
    static uint8_t const SW_B_LED_PIN = 11;
    static uint8_t const SW_C_INPUT_PIN = 17;
    static uint8_t const SW_C_LED_PIN = 12;
    static uint8_t const SW_D_INPUT_PIN = 16;
    static uint8_t const SW_D_LED_PIN = 13;
    static uint8_t const SW_E_INPUT_PIN = 9;
    static uint8_t const SW_E_LED_PIN = 5;
    static uint8_t const SW_F_INPUT_PIN = 19;
    static uint8_t const SW_F_LED_PIN = 6;
    static uint8_t const SW_POWER_PIN = 20;
    static uint8_t const SW_TOGGLE_PIN = 21;
    static uint8_t const DIGITAL_PIN_COUNT = 8;
    static uint8_t const pinmap[DIGITAL_PIN_COUNT];
    static uint8_t const POLLRATE = 250;
    uint32_t timer;

    uint8_t sw_states[DIGITAL_PIN_COUNT];
    io_callback callbacks[DIGITAL_PIN_COUNT];
};

uint8_t const IO::pinmap[] = {
  IO::SW_A_INPUT_PIN,
  IO::SW_B_INPUT_PIN,
  IO::SW_C_INPUT_PIN,
  IO::SW_D_INPUT_PIN,
  IO::SW_E_INPUT_PIN,
  IO::SW_F_INPUT_PIN,
  IO::SW_POWER_PIN,
  IO::SW_TOGGLE_PIN
};

IO *io;

void selectAnimation(uint8_t animation) {
  delete current_animation;
  Serial.print("Setting anim "); Serial.println(animation); 
  switch(animation) {
    default:
    case 0:
      current_animation = new Cylon();
      break;
    case 1:
      current_animation = new RandomArcs();
      break;
    case 2:
      current_animation = new Random();
      break;
    case 3:
      current_animation = new Rotators();
      break;
    case 4:
    case 5:
      current_animation = new Cylon();
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  brightness = 32;
  light_enable = true;
  power_enable = true;
  io = new IO();
  io->set_callback(0, [](bool state) {if(!state)selectAnimation(0);});
  io->set_callback(1, [](bool state) {if(!state)selectAnimation(1);});
  io->set_callback(2, [](bool state) {if(!state)selectAnimation(2);});
  io->set_callback(3, [](bool state) {if(!state)selectAnimation(3);});
  io->set_callback(4, [](bool state) {if(!state)selectAnimation(4);});
  io->set_callback(5, [](bool state) {if(!state)selectAnimation(5);});
  //Switches
  io->set_callback(6, [](bool state) {light_enable = state;});
  io->set_callback(7, [](bool state) {
    power_enable = !state;
    if (state){
      FastLED.clear();
      io->clearLEDS();
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
    io->update();
  }
}
