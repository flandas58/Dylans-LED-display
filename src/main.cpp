#include <Arduino.h>

#include <Adafruit_NeoPixel.h>

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>


#ifdef __AVR__
  #include <avr/power.h>
#endif

#define ledDataPin 5 // GPIO pin 5 (D1 on a NodeMCU board).
// An IR detector/demodulator is connected to GPIO pin 4 (D2 on a NodeMCU board).
const uint16_t kRecvPin = 4;

IRrecv irrecv(kRecvPin);

decode_results results;

const int  CYCLE_MILLISECONDS = 5000; // 5 second breathing cycle.

const int striplength = 62; // number of leds on strip

const uint8_t KEYFRAMES[]  = {
  // Rising
  20, 21, 22, 24, 26, 28, 31, 34, 38, 41, 45, 50, 55, 60, 66, 73, 80, 87, 95,
  103, 112, 121, 131, 141, 151, 161, 172, 182, 192, 202, 211, 220, 228, 236,
  242, 247, 251, 254, 255,

  // Falling
  254, 251, 247, 242, 236, 228, 220, 211, 202, 192, 182, 172, 161, 151, 141,
  131, 121, 112, 103, 95, 87, 80, 73, 66, 60, 55, 50, 45, 41, 38, 34, 31, 28,
  26, 24, 22, 21, 20,
  20, 20, 20, 20, 20, 20, 20, 20, 20, 20
};

const uint32_t BASECOLORS[] = {
  20, 21, 22, 24, 26, 28, 31, 34, 38, 41, 45, 50, 55, 60, 66, 73, 80, 87, 95,
  103, 112, 121, 131, 141, 151, 161, 172, 182, 192, 202, 211, 220, 228, 236,
  242, 247, 251, 254, 255
};

// variables used by breathe

unsigned long lastBreath = 0.0;
int keyframePointer = 0;
uint32_t breatheLength = striplength/3;
uint32_t cyclecount = 0;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(striplength, ledDataPin, NEO_GRB + NEO_KHZ800);

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
  // Start methods

  void breathe(uint32_t midColor) {
    int numKeyframes = sizeof(KEYFRAMES) - 1;
    float period = CYCLE_MILLISECONDS / numKeyframes;
    unsigned long now = millis();

    if ((now - lastBreath) > period) {

      lastBreath = now;
      for (uint32_t i = breatheLength; i < strip.numPixels() - breatheLength; i++){
        strip.setPixelColor(i, midColor);
      }

      for (uint32_t i = 0; i < breatheLength; i++) {
        uint8_t color = (127 * KEYFRAMES[keyframePointer]) / 256;
        strip.setPixelColor(i, color, 0, color);
        strip.setPixelColor(strip.numPixels() - breatheLength + i, color, 0, color);
      }


      strip.show();

      // Increment the keyframe pointer.
      if (++keyframePointer > numKeyframes) {
        // Reset to 0 after the last keyframe.
        keyframePointer = 0;
      }
    }
  }

  void rainbow(uint8_t wait) {
    uint16_t i, j;

    for(j=0; j<256; j++) {
      for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel((i+j) & 255));
      }
      strip.show();
      delay(wait);
    }
  }

  // Slightly different, this makes the rainbow equally distributed throughout
  void rainbowCycle(uint8_t wait) {
    uint16_t i, j;

    for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
      for(i=0; i< strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      }
      strip.show();
      delay(wait);
    }
  }

  //Theatre-style crawling lights with rainbow effect
  void theaterChaseRainbow(uint8_t wait) {
    for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
      for (int q=0; q < 3; q++) {
        for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();


        delay(wait);

        for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
      }
    }
  }

void colorWipe(uint32_t c, uint8_t _wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    // wait before next pixel
    delay(_wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  Serial.begin(115200);
  irrecv.enableIRIn();  // Start the receiver

  strip.Color(0, 0, 0);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

}

void loop() {

  if (irrecv.decode(&results)) {
    String hexResults = uint64ToString(results.value, HEX);
    // print() & println() can't handle printing long longs. (uint64_t)
    if (hexResults.length() == 6) {
      //Serial.println(hexResults);
      if ( strip.getBrightness() > 10){
        strip.setBrightness(strip.getBrightness() - 10);
      } else {
        strip.setBrightness(0);
      }
    }
    irrecv.resume();  // Receive the next value
  }

  // Some example procedures showing how to display to the pixels:


  colorWipe(strip.Color(32, 0, 0), 50); // Red
  colorWipe(strip.Color(32, 32, 0), 50);
  colorWipe(strip.Color(0, 32, 0), 50); // Green
  colorWipe(strip.Color(0, 32,32), 50);
  colorWipe(strip.Color(0, 0, 32), 50); // Blue
    // custom color??
    //  colorWipe(strip.Color(0, 0, 127), 50); // Blue

 // cyclecount++;
 // if (cyclecount < 6){
 //   breathe( strip.Color(32, 32, 32) );
 // } else if (cyclecount < 11){
 //   breathe( strip.Color(32, 32, 32) );
 // } else if (cyclecount < 16){
 //   breathe( strip.Color(32, 32, 32) );
 // } else {
 //   cyclecount = 0;
 // }

//colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW


  // Send a theater pixel chase in...
//  theaterChase(strip.Color(127, 127, 127), 50); // White
//  theaterChase(strip.Color(127, 0, 0), 50); // Red
//  theaterChase(strip.Color(0, 0, 127), 50); // Blue

//  rainbow(20);
//  rainbowCycle(20);
//  theaterChaseRainbow(50);
  delay(10);
}
