/*
 * Wrappers to make I/O functions available with C linkage. This allows C++
 * methods to be called from C code.
 *
 * Copyright 2011-2017 The MathWorks, Inc. */

#include "Arduino.h"
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "rtwtypes.h"

#define BACKGROUND_COLOR ILI9341_BLUE
#define TFT_CS 10
#define TFT_DC 9

unsigned int ColorArray[] = {ILI9341_BLACK,
                             ILI9341_NAVY,
                             ILI9341_DARKGREEN,
                             ILI9341_DARKCYAN,
                             ILI9341_MAROON,
                             ILI9341_PURPLE,
                             ILI9341_OLIVE,
                             ILI9341_LIGHTGREY,
                             ILI9341_DARKGREY,
                             ILI9341_GREEN,
                             ILI9341_CYAN,
                             ILI9341_RED,
                             ILI9341_MAGENTA,
                             ILI9341_YELLOW,
                             ILI9341_WHITE,
                             ILI9341_ORANGE,
                             ILI9341_GREENYELLOW,
                             ILI9341_PINK,
                             ILI9341_BLUE};

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
bool config = false;

/* TFT initialization*/
extern "C" void tftSetup(void){
    if(!config){
        tft.begin();
        tft.fillScreen(BACKGROUND_COLOR);
        tft.setRotation(3);
        config = true;
    }
}

/* Set Text Size */
extern "C" void setTextSize(int size){
    tft.setTextSize(size);
}

/* Get TFT Width */
extern "C" unsigned int getTftWidth(void){
    return tft.width();
}

/* Get TFT Height */
extern "C" unsigned int getTftHeight(void){
    return tft.height();
}
/* Set Tft Fill Screen */
extern "C" void setTftFillscreen(void){
    tft.fillScreen(BACKGROUND_COLOR);
}

/* Display Heart Rate at specified (x,y) position */
extern "C" void displayHeartRate(int txtcolor, int heart_rate, uint_T x, uint_T y){
	  static int prev_heart_rate;	
      tft.setTextColor(BACKGROUND_COLOR);  
	  tft.setCursor(x, y);
	  tft.println(prev_heart_rate);
      tft.setTextColor(ColorArray[txtcolor]);  
      tft.setCursor(x, y);
      tft.println(heart_rate);
	  prev_heart_rate = heart_rate; 
}

/* Draw a line */
extern "C" void DrawTftLine(uint_T x1, uint_T y1, uint_T x2, uint_T y2, int linecolor){
    tft.drawLine(x1, y1, x2, y2, ColorArray[linecolor]);
}