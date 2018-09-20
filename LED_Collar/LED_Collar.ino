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

uint8_t color_index = 0;
CRGB colors[] = {
  CRGB::Red,
  CRGB::Blue,
  CRGB::Green,
  CRGB::DarkViolet
};

CRGB currentColor() {
  return colors[color_index];
}

uint16_t global_rate;
#define GLOBAL_RANGE 195
#define MIN_GLOBAL_RATE 5

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
      if ((millis() - timer) > (global_rate)) {
        timer = millis();
        leds[i] = currentColor();
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

class Random : public Animation {
  public:
    Random() {
      for(uint8_t i = 0; i < LED_RING_COUNT; ++i) {
        state[i] = random(0,2);
        timers[i] = millis();
      }
    }
  
    void Draw() {
      for(uint8_t i = 0; i < LED_RING_COUNT; ++i) {
        if (millis() > timers[i]) {
          state[i] = !state[i];
          timers[i] = random(MIN_PERIOD, 10 * global_rate) + millis();
        }
        leds[i] = state[i] ? currentColor() : CRGB(0,0,0);
      }
      FastLED.show();
    }
    
  private:
    static const uint32_t MIN_PERIOD = 5;
    uint32_t timers[LED_RING_COUNT];
    bool state[LED_RING_COUNT];
};

class Rotators : public Animation {
  public:
    Rotators() {
      offset = 0;
      timer = millis();
      uint16_t e = 4;
      uint16_t j = 0;
      uint8_t i = 0;
      bool state = true;
      while(i < LED_RING_COUNT) {
        if (j < e) {
          ring_buffer[i] = true;
        } else {
          ring_buffer[i] = false;
        }
        ++i;
        ++j;
        if (j >= (2*e)) {
          e += 4;
          j = 0;
        }
      }
    }

    void Draw() {
      if ((millis() - timer) > (global_rate)) {
        timer = millis();
        uint8_t i = offset;
        uint8_t j = 0;
        for(uint8_t j = 0; j < LED_RING_COUNT; ++j) {
          if (ring_buffer[i] == true) {
            leds[j] = currentColor();
          } else {
            leds[j] = CRGB::Black;
          }
          ++i;
          if (i >= LED_RING_COUNT) {
            i = 0;
          }
        }
        ++offset;
        if (offset >= LED_RING_COUNT) {
          offset = 0;
        }
        FastLED.show();
      }
    }
  private:
    uint8_t offset;
    bool ring_buffer[LED_RING_COUNT];
    static const uint32_t RATE = 30;
};

Animation* current_animation = NULL;

typedef void(*io_callback)(bool io);

typedef enum {
  LED_SOLID = 0,
  LED_RANDOM,
  LED_PULSE,
  LED_CLEAR
} FrontPattern;

class IO {
  public:
    IO() {
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
      for(uint8_t i = 0; i < LED_PIN_COUNT; ++i) {
        led_timer[i] = millis();
        led_value[i] = LEDBRIGHTNESS;
      }
      poll_timer = millis();
      setSolid();
      update();
      
    };

    void resetTimers() {
      for(uint8_t i = 0; i < LED_PIN_COUNT; ++i) {
        led_timer[i] = millis();
      }
    }

    void setSolid() {
      led_state = LED_SOLID;
      resetTimers();
    };
    
    void setPulse() {
      led_state = LED_PULSE;
      resetTimers();
    };
    
    void setRandom() {
      led_state = LED_RANDOM;
      resetTimers();
    };

    void clearLEDS() {
      led_state = LED_CLEAR;
      resetTimers();
    };

    void set_callback(uint8_t pin, io_callback callback) {
      callbacks[pin] = callback;
    };
  
    void edge_detect() {
      for(uint8_t i =0; i < DIGITAL_PIN_COUNT; ++i) {
        uint8_t value = digitalRead(pinmap[i]);
        if (value != sw_states[i]) {
          sw_states[i] = value;
          
          if(callbacks[i] != NULL)
            callbacks[i](value);
        }
      }
    };

    void update() {
      if ((millis() - poll_timer) > POLLRATE) {
        poll_timer = millis();
        edge_detect();
        uint16_t pot_value = analogRead(POT_INPUT_PIN);
        // Pot value is clamped to between 0 and 512
        if (pot_value > 511)
          pot_value = 511;
        if (pot_value <= 20) 
          pot_value = 0;
        global_rate = MIN_GLOBAL_RATE + ((GLOBAL_RANGE * pot_value) / 511);
      }
      for(uint8_t i = 0; i < LED_PIN_COUNT; ++i) {
        switch(led_state) {
          case LED_SOLID:
            if (led_value[i] != LEDBRIGHTNESS) {
              led_value[i] = LEDBRIGHTNESS;
              analogWrite(ledmap[i], led_value[i]);
            }
            break;
          case LED_PULSE:
            {
              uint32_t delta = millis() - led_timer[i];
              if (delta < 2000) {
                led_value[i] = (LEDBRIGHTNESS * delta) / 2000;
              } else if (delta < 2500) {
                //led_value[i] = LEDBRIGHTNESS; 
              } else if (delta < 4500) {
                led_value[i] = (LEDBRIGHTNESS * (2000 - (delta - 2500))) / 2000;
              } else if (delta < 5250) {
                //led_value[i] = 0;
              } else {
                led_timer[i] = millis();
              }
              analogWrite(ledmap[i], led_value[i]);
            }
            break;
          case LED_RANDOM:
            if (led_timer[i] < millis()) {
              led_timer[i] = millis() + random(300, 2500);
              if (led_value[i] == 0) {
                led_value[i] = LEDBRIGHTNESS;
                analogWrite(ledmap[i], led_value[i]);
              } else {
                led_value[i] = 0;
                analogWrite(ledmap[i], led_value[i]);
              }
            }
            break;
          case LED_CLEAR:
            analogWrite(ledmap[i], 0);
            break;
          default:
            break;
        }
      }
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
    static uint8_t const LED_PIN_COUNT = 6;
    static uint8_t const pinmap[DIGITAL_PIN_COUNT];
    static uint8_t const ledmap[LED_PIN_COUNT];
    static uint8_t const POLLRATE = 250;
    static uint8_t const LEDBRIGHTNESS = 128;
    uint32_t poll_timer;

    uint8_t sw_states[DIGITAL_PIN_COUNT];
    io_callback callbacks[DIGITAL_PIN_COUNT];

    uint8_t led_value[6];
    uint32_t led_timer[6];
    uint8_t led_state;
};

uint8_t const IO::pinmap[] = {
  IO::SW_A_INPUT_PIN,
  IO::SW_B_INPUT_PIN,
  IO::SW_C_INPUT_PIN,
  IO::SW_E_INPUT_PIN,
  IO::SW_F_INPUT_PIN,
  IO::SW_D_INPUT_PIN,
  IO::SW_POWER_PIN,
  IO::SW_TOGGLE_PIN
};

uint8_t const IO::ledmap[] = {
  IO::SW_A_LED_PIN,
  IO::SW_B_LED_PIN,
  IO::SW_C_LED_PIN,
  IO::SW_D_LED_PIN,
  IO::SW_E_LED_PIN,
  IO::SW_F_LED_PIN
};

IO *io;

void selectAnimation(uint8_t animation) {
  delete current_animation;
  switch(animation) {
    default:
    case 0:
      current_animation = new Cylon();
      break;
    case 1:
      current_animation = new Rotators();
      break;
    case 2:
      current_animation = new Random();
      break;  
  }
}

void setup() {
  //Serial.begin(115200);
  //Serial.println("Booting");
  brightness = 32;
  light_enable = true;
  power_enable = true;
  io = new IO();
  io->set_callback(0, [](bool state) {if(!state)selectAnimation(0);});
  io->set_callback(1, [](bool state) {if(!state)selectAnimation(1);});
  io->set_callback(2, [](bool state) {if(!state)selectAnimation(2);});
  io->set_callback(3, [](bool state) {if(!state)io->setSolid();});
  io->set_callback(4, [](bool state) {if(!state)io->setPulse();});
  io->set_callback(5, [](bool state) {if(!state)io->setRandom();});
  //Switches
  io->set_callback(6, [](bool state) {
    power_enable = !state;
    if (!power_enable){
      io->clearLEDS(); 
    }
  });
  io->set_callback(7, [](bool state) {
    ++color_index;
    if (color_index >= (sizeof(colors)/sizeof(CRGB))) {
      color_index = 0;
    }
  });
  FastLED.addLeds<APA102, BGR>(leds, LED_RING_COUNT);
  FastLED.setBrightness(brightness);
  selectAnimation(1);
  io->setRandom();
  color_index = 0;
  //Serial.println("Complete");
}

void loop() {
  if (power_enable) {
    if (current_animation != NULL) {
      current_animation->Draw();
    } 
  } else {
    FastLED.clear();
    for(uint8_t i = 0; i < LED_RING_COUNT; ++i) {
      leds[i] = CRGB::Black;
    }
    FastLED.show();
  }
  io->update();
}
