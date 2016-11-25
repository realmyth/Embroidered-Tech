#include "FastLED.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

//Output Pins
#define DATA_PIN    11
#define CLK_PIN   10

//input pins
const int inputPinInner = A2;              // the input front the inner ring
const int inputPinOuter = A1;              // the input front the outer ring

//Fast LED Stuff
#define LED_TYPE    APA102
#define COLOR_ORDER RGB
#define NUM_LEDS    45
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

//noise reduction and smoothing constants
const int numReadings = 10;
const int errorRange = 10;
const long errorRangeVoltage = .5;
const int noiseMinimum = 50;

// Smoothing Magic
int readingsInner[numReadings];      // the readings from the analog input
int readIndexInner = 0;              // the index of the current reading
int totalInner = 0;                  // the running total
int averageInner = 0;                // the average
float voltageInner;                  // the calculated voltage for 5v
float previousVoltageInner = 0;      // trying to catch random jumps


int readingsOuter[numReadings];      // the readings from the analog input
int readIndexOuter = 0;              // the index of the current reading
int totalOuter = 0;                  // the running total
int averageOuter = 0;                // the average
float voltageOuter;                  // the calculated voltage for 5v
float previousVoltageOuter = 0;      // trying to catch random jumps



// the setup routine runs once when you press reset:
void setup() {
  delay(3000); // 3 second delay for recovery

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readingsInner[thisReading] = 0;
    readingsOuter[thisReading] = 0;
  }
}

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

// the loop routine runs over and over again forever:
void loop() {
  // subtract the last reading:
  /* Serial.println(analogRead(inputPinInner));
   Serial.println(analogRead(inputPinOuter));
   Serial.println();*/
  totalInner = totalInner - readingsInner[readIndexInner];
  totalOuter = totalOuter - readingsOuter[readIndexInner];
  // read from the sensor:
  readingsInner[readIndexInner] = analogRead(inputPinInner);
  readingsOuter[readIndexOuter] = analogRead(inputPinOuter);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  voltageInner = readingsInner[readIndexInner] * (5.0 / 1023.0);
  voltageOuter = readingsOuter[readIndexOuter] * (5.0 / 1023.0);
  // add the reading to the total:
  totalInner = totalInner + readingsInner[readIndexInner];
  totalOuter = totalOuter + readingsOuter[readIndexOuter];
  // advance to the next position in the array:
  readIndexInner = readIndexInner + 1;
  readIndexOuter = readIndexOuter + 1;

  // if we're at the end of the array...
  if (readIndexInner >= numReadings)
    // ...wrap around to the beginning:
    readIndexInner = 0;
    
  // if we're at the end of the array...
  if (readIndexOuter >= numReadings)
    // ...wrap around to the beginning:
    readIndexOuter = 0;  

  // calculate the average:
  averageInner = totalInner / numReadings;
  averageOuter = totalOuter / numReadings;
  


  /*if (readingsInner[readIndexInner] < averageInner + errorRange && readingsInner[readIndexInner] > averageInner - errorRange) {
   // if (voltageInner < previousVoltageInner + errorRangeVoltage && voltageInner > previousVoltageInner - errorRangeVoltage) {
      Serial.print("Inner ring: ");
      Serial.println(voltageInner);
    //  }
    previousVoltageInner = voltageInner;
    Serial.println("as");
  }*/
  if (readingsOuter[readIndexOuter] < averageOuter + errorRange && readingsOuter[readIndexOuter] > averageOuter - errorRange) {
    Serial.print("Outer ring: ");
    Serial.println(voltageOuter);
    if (averageOuter >= noiseMinimum) {
      gHue = averageOuter * (256.0 / 1023.0);
    }
  }

  // Call the current pattern function once, updating the 'leds' array
  rainbow();
  
  FastLED.show();
  delay(1000/FRAMES_PER_SECOND);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 1);
}
