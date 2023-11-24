// * ECG displayed on Adafruit 2.8" LCD v.1 * // 
// * using Arduino UNO board * //
// * Written by S.Van Voorst * //

// ************************************************************ //
// *  This source code is provided by the author "AS IS",   * //
// *  without warranty of any kind, either express or implied.  * //
// ************************************************************ //

// Serial Peripheral Interface
// http://arduino.cc/en/Reference/SPI
#include "SPI.h"
#include "Adafruit_GFX.h"    // Core graphics library
#include "Adafruit_ILI9341.h" // Hardware-specific library
#include "Timer.h"

extern "C" {
#include "heart_rate_detector_arduino_interface.h"
}

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

#define BACKGROUND_COLOR ILI9341_BLUE
#define TEXT_COLOR ILI9341_WHITE
#define LINE_COLOR ILI9341_WHITE

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Timer processSignalTimer;
Timer signalDisplayTimer;
Timer sensorDisplayTimer;

// Determined by AIN_SEL setting on A5 on Olimex, or A0 on BackyardBrains
const int analogInPin = A0;

// If BUFFER_LENGTH is around 640, then there is not enough memory.
// You can have a local variable of this size, but not a global.
const int BUFFER_LENGTH = 1;
int buffer[BUFFER_LENGTH];

int counter = 0;
int rawSensorValue = 0;
int sensorValue = 0;
float x1_global, y1_global, x2_global, y2_global;
// spacing = 0.25 is about 10 seconds across the screen
const float spacing = 0.25;
int heart_rate = 0;
int previous_heart_rate = 0;


void processSignal(void)
{
  // analogRead maps input voltages between 0 and 5 volts into 
  // integer values between 0 and 1023 (unsigned 10 bit value,
  // returned in an int == int16 in the lower 10 bits.
  // An int stores a 16-bit (2-byte) value (int16)
  rawSensorValue = analogRead(analogInPin);
  sensorValue = rawSensorValue;
  heart_rate = heart_rate_detector_arduino_interface(rawSensorValue);

}

void displaySignal(void)
{
  //sensorValue = filtered_signal;
  sensorValue += 500;

  // tft.width() = 320
  // spacing = 0.5
  // tft.width()/spacing = 640
  if (counter > tft.width()/spacing) { 
    counter = 0; 
    // I think fillScreen takes long enough that I'm missing samples
    // because the heart rate measurement is always messed up after
    // fillScreen fires.  If you comment-out fillScreen, then the 
    // heart rate is consistent, but the trace keeps going back over
    // itself.  
    // Try overwriting the trace by reverting the color like I do in
    // displaying the heart rate.  It would require a vector of previous
    // values.
    tft.fillScreen(BACKGROUND_COLOR); 

  }
  sensorValue = sensorValue >> 3;
  if (counter == 0) { x1_global = 0; y1_global = tft.height() - sensorValue; }
  if (counter > 0) {
    x2_global = counter*spacing;
    //  To make it right side up
    y2_global = tft.height() - sensorValue;
    tft.drawLine(x1_global, y1_global, x2_global, y2_global, LINE_COLOR);
    x1_global = x2_global;
    y1_global = y2_global;
  }
  counter++;
}


void displayHeartRate(void)
{
  // Set the cursor position before every print
  // Overwrite the old value with the background color
  if (previous_heart_rate != heart_rate) {
      tft.setTextColor(BACKGROUND_COLOR);  
      tft.setCursor(0, 0);
      tft.println(previous_heart_rate);
      tft.setTextColor(TEXT_COLOR);  
      tft.setCursor(0, 0);
      tft.println(heart_rate);
      previous_heart_rate = heart_rate;
    }
}

void setup()
{
  tft.begin();
  tft.fillScreen(BACKGROUND_COLOR);
  // setRotation(3) = landscape, right is the plugs
  // This makes it so I can see it right-side up when 
  // plugged into the USB ports on the left of my laptop.
  tft.setRotation(3);
  tft.setTextSize(5);
  heart_rate_detector_arduino_interface_initialize();

  // Every n milliseconds.  5 ms = 200 samples/second = Fs of MIT database
  processSignalTimer.every(5, processSignal);
  signalDisplayTimer.every(7, displaySignal);
  sensorDisplayTimer.every(59, displayHeartRate);
  for (int n=0; n<BUFFER_LENGTH; n++) { buffer[n] = 0; }
}

void loop()
{
  processSignalTimer.update();
  signalDisplayTimer.update();
  sensorDisplayTimer.update();
}
