#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 4
#define BUTTON 3

static uint16_t random_number = 0;

static uint16_t lfsr16_next(uint16_t n)
{ 
  return (n >> 0x01U) ^ (-(n & 0x01U) & 0xB400U);    
}

uint16_t random_t()
{
  random_number = lfsr16_next(random_number);
  return random_number; 
}

Adafruit_NeoPixel strip = Adafruit_NeoPixel(2, PIN, NEO_GRBW + NEO_KHZ800);
unsigned int Step = 0;
uint8_t Mode = 0;
uint8_t Num_Modes = 11;
bool Switch_On = false;

void setup() {
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH);
  random_number = analogRead(A0);
  
  strip.begin();
  strip.setBrightness(30);
  strip.show(); // Initialize all pixels to 'off'
}

void check_button_state()
{
  if (digitalRead(BUTTON)==LOW)
  {
    if (!Switch_On)
    {
      Switch_On = true;
      Step = 0;
      Mode++;
      Mode = Mode % Num_Modes; // constrain
    }
  }
  else
  {
    Switch_On = false;
  }
}

const uint32_t Flasher[2][4] PROGMEM = {
  {0xFF0000, 0x000000, 0x0000FF, 0x000000},
  {0x000000, 0x0000FF, 0x000000, 0xFF0000},
  };

const uint8_t Heart_Size = 26;
const uint8_t Heart[Heart_Size] PROGMEM = {0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0,
                                           0xB0, 0x90, 0x60, 0x90, 0xB0, 
                                           0xE0, 0xC0, 0xA0, 0x80, 0x60, 0x40, 0x20, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00};

void loop() {
  check_button_state();
 
  switch (Mode)
  { 
    case 0:
      // Slow rainbow fade
      rainbowFade(200, 16);
      break;
    case 1:
      // Fast rainbow fade
      rainbowFade(50, 16);
      break;
    case 2:
      // Police flasher #1
      updateFlasher(100);
      break;
    case 3:
      // Police flasher #2
      updateFlasher2(100);
      break;
    case 4:
      // Solid white
      solidColor(strip.Color(0, 0, 0, 255), 50);
      break;
    case 5:
      // Solid red
      solidColor(strip.Color(255, 0, 0), 50);
      break;
    case 6:
      // Solid green
      solidColor(strip.Color(0, 255, 0), 50);
      break;
    case 7:
      // Solid blue
      solidColor(strip.Color(0, 0, 255), 50);
      break;
    case 8:
      // Beating heart
      solidColor(strip.Color(pgm_read_byte(&Heart[(Step % Heart_Size)]),0,0), 100);
      break;
    case 9:
    {
      // Fade up-down red
      uint8_t value = Step % 32;
      if (value > 15)
      {
        value = 31 - value; 
      }
      solidColor(strip.Color(value<<0x4,0,0), 100);
      break;
    }
    case 10:
      // Random color
      randomColor(500);
      break;
  }
  Step++;
  // Prevent overflow
  Step = Step % 32768;
}

void solidColor(uint32_t c, uint8_t wait)
{
  for(uint8_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  } 
  strip.show(); 
  delay(wait);
}

void updateFlasher(uint8_t wait)
{
  // Flasher has 
  for(uint8_t i=0; i<strip.numPixels(); i++) {
    uint32_t value;

    if(Step%2==1)
    {
      value = strip.Color(0,0,0);
    }
    else
    {
      if(Step%16 < 8)
      {
        value = pgm_read_dword(&Flasher[i%2][0]);
      }
      else
      {
        value = pgm_read_dword(&Flasher[i%2][1]);
      }
    }
    strip.setPixelColor(i, value);  
  } 
  strip.show(); 
  delay(wait);  
}

void updateFlasher2(uint8_t wait)
{
  for(uint8_t i=0; i<strip.numPixels(); i++) {
    if(Step%16 < 8)
    {
      strip.setPixelColor(i, pgm_read_dword(&Flasher[i%2][Step%2]));
    }
    else
    {
      strip.setPixelColor(i, pgm_read_dword(&Flasher[i%2][Step%2+2]));
    }
  } 
  strip.show(); 
  delay(wait);  
}

void randomColor(uint8_t wait)
{
  for(uint8_t i=0; i<strip.numPixels(); i++) {
     strip.setPixelColor(i, getRandomColor());
  } 
  strip.show(); 
  delay(wait);  
}

uint32_t getRandomColor()
{
  uint8_t r = (random_t() % 5) * 64;
  uint8_t g = (random_t() % 5) * 64;
  uint8_t b = (random_t() % 5) * 64;
  return strip.Color(r,g,b);
}

void rainbowFade(uint8_t wait, uint8_t spread) {
  uint8_t i;
  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel((Step + i * spread) % 255));
  }
  strip.show();
  delay(wait);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
