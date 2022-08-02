/* 3D Printer Bed Leveler
 * Copyright (C) 2020 by Dominick Lee (http://dominicklee.com)
 * 
 * Last Modified Dec, 2020.
 * This program is free software: you can use it, redistribute it, or modify
 * it under the terms of the MIT license (See LICENSE file for details).
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR 
 * ANY CLAIM, DAMAGES,OR OTHER LIABILITY. YOU AGREE TO USE AT YOUR OWN RISK.
 * 
*/

/*
I modified this program, in order to easily switch from M5StickC to M5StickCplus.
For this it is enough to comment lines or comment them.
To achieve this I have created variables that allow to move from one to the other.
Do not forget to load in the Arduino editor to set the card type to use M5StickC or M5StickCplus.
*/


// For M5StickC Comment for M5StickCplus or uncomment line if you will use this for M5StickC
// Begin M5StickC bloc

#include <M5StickC.h>
#include <EEPROM.h>
#include "OneButton.h"
#include "rotate-left.h"
#include "rotate-right.h"
#include "thumbs-up.h"
int set_point = 64;
int rect_width = 21;
int rect_height = 133;
int x_off = 42;
int y_off = 75;
int x_cursor = 15;
int y_cursor = 150;
int pressure_out = 127;
int text_size = 1;
int x_image = 40;
int x_size_image = 32;
int y_size_image = 32;
int x_calibration = 26;
int x_calibration_end = 29;
int x_progressbar = 142;
int x_progressbar_width = 15;
int adjust_color = 32;

// End M5StickC bloc
/*
// for M5StickCplus comment for M5StickC or uncomment Bloc line if you will use this for M5StickCplus
// Begin M5StickCplus bloc
#include <M5StickCPlus.h>
#include <EEPROM.h>
#include "OneButton.h"
#include "rotate-leftPlus.h"
#include "rotate-rightPlus.h"
#include "thumbs-upPlus.h"
int set_point = 92; // global 
int rect_width = 45;
int rect_height = 190;
int x_off = 60;
int y_off = 113;
int x_cursor = 20;
int y_cursor = 220;
int pressure_out = 184;
int text_size = 2;
int x_image = 70;
int x_size_image = 48;
int y_size_image = 48;
int x_calibration = 50;
int x_calibration_end = 54;
int x_progressbar = 199;
int x_progressbar_width = 40;
int adjust_color = 46;
// end M5StickCPlus bloc
*/
// Icons converted at: 
// http://rinkydinkelectronics.com/_t_doimageconverter565.php

#define EEPROM_SIZE 1
int ledPin = G10;
int pressure = 0;   //the var that goes between 0-127 from FSR reading
int setpoint = set_point;  //to be updated by EEPROM
float errorThreshold = 6.0; //printbed is considered leveled if error goes below this (ideally 5.0-8.0)

//Object initialization
OneButton btnA(G37, true);
OneButton btnB(G39, true);

void setup() {
  M5.begin();
  EEPROM.begin(EEPROM_SIZE);  //Initialize EEPROM
  EEPROM.write(0, setpoint);  // save in EEPROM
  EEPROM.commit();
  Serial.begin(115200);       //Initialize Serial
  pinMode(ledPin, OUTPUT);    //Set up LED
  digitalWrite (ledPin, HIGH); // turn off the LED

  M5.Lcd.setRotation(0);
  M5.Lcd.fillScreen(TFT_BLACK);

  btnA.attachClick(btnAClick);  //BtnA handle
  btnA.setDebounceTicks(40);
  btnB.attachClick(btnBClick);  //BtnB handle
  btnB.setDebounceTicks(25);
  
  //M5.Lcd.drawRect(5, 12, 21, 133, 0x7bef);  //show frame for progressbar (X, Y Width, height, color) // old line
  M5.Lcd.drawRect(5, 12, rect_width, rect_height, 0x7bef);  //show frame for progressbar (X, Y Width, height, color)
  getCalibration(); //show calibration mark

  //Show OFF instructions
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setTextSize(text_size);  // text size
  // M5.Lcd.setCursor(42, 75); //off position X, Y // old line
  M5.Lcd.setCursor(x_off, y_off); //off position X, Y
  M5.Lcd.printf("Off ->");

  //Show Calibrate instructions
  M5.Lcd.setTextColor(RED);
  // M5.Lcd.setCursor(15, 150);  // calibrate position X, Y // old line
  M5.Lcd.setCursor(x_cursor, y_cursor);  // calibrate position X, Y
  M5.Lcd.printf("Calibrate");

  // Swap the colour byte order when rendering
  M5.Lcd.setSwapBytes(true);
}

void loop() {
  //poll for button press
  btnA.tick();
  btnB.tick();
  
  //pressure = map(analogRead(G36), 0, 4095, 0, 127); //get reading input number, begin range read, end range read, begin range output, end range output // old line
  pressure = map(analogRead(G36), 0, 4095, 0, pressure_out); //get reading input number, begin range read, end range read, begin range output, end range output
  Serial.println(pressure);
  
  progressBar(pressure);  //show reading on progressbar
  
  if (getPercentError(pressure, setpoint) < errorThreshold) {
    setLED(true);
    // M5.Lcd.pushImage(40, 15, 32, 32, thumbs_up);  //Draw icon position X, Y ,image size x, y // old line
    M5.Lcd.pushImage(x_image, 15, x_size_image, y_size_image, thumbs_up);  //Draw icon position X, Y ,image size x, y
  } else {
    setLED(false);
    if (pressure > setpoint) {
      // M5.Lcd.pushImage(40, 15, 32, 32, rotate_left);  //Draw icon position X, Y ,image size x, y // old line
      M5.Lcd.pushImage(x_image, 15, x_size_image, y_size_image, rotate_left);  //Draw icon position X, Y ,image size x, y
    } else {
      // M5.Lcd.pushImage(40, 15, 32, 32, rotate_right);  //Draw icon position X, Y ,image size x, y // old line
      M5.Lcd.pushImage(x_image, 15, x_size_image, y_size_image, rotate_right);  //Draw icon position X, Y ,image size x, y
    }
  }
  delay(50);
}

void btnBClick()
{
  M5.Axp.PowerOff();
}

void btnAClick()
{
  updateCalibration(pressure);
}

float getPercentError(float approx, float exact)
{
  return (abs(approx-exact)/exact)*100;
}

void updateCalibration(int value)
{
  EEPROM.write(0, value);  // save in EEPROM
  EEPROM.commit();
  // clear old line
  // M5.Lcd.drawLine(1, 15+(127-setpoint), 4, 15+(127-setpoint), BLACK);  // draw line from X, Y to x, y // old line
  // M5.Lcd.drawLine(26, 15+(127-setpoint), 29, 15+(127-setpoint), BLACK); // draw line from X, Y to x, y // old line
  M5.Lcd.drawLine(1, 15+(pressure_out-setpoint), 4, 15+(pressure_out-setpoint), BLACK);  // draw line from X, Y to x, y
  M5.Lcd.drawLine(x_calibration, 15+(pressure_out-setpoint), x_calibration_end, 15+(pressure_out-setpoint), BLACK); // draw line from X, Y to x, y
  // set new line
  setpoint = value; //set global
  // M5.Lcd.drawLine(1, 15+(127-setpoint), 4, 15+(127-setpoint), 0x7bef); // draw line from X, Y to x, y // old line
  // M5.Lcd.drawLine(26, 15+(127-setpoint), 29, 15+(127-setpoint), 0x7bef); // draw line from X, Y to x, y // old line
  M5.Lcd.drawLine(1, 15+(pressure_out-setpoint), 4, 15+(pressure_out-setpoint), 0x7bef); // draw line from X, Y to x, y
  M5.Lcd.drawLine(x_calibration, 15+(pressure_out-setpoint), x_calibration_end, 15+(pressure_out-setpoint), 0x7bef); // draw line from X, Y to x, y
}

void getCalibration()
{
  setpoint = EEPROM.read(0);  // retrieve calibration in EEPROM
  // set new line
  // M5.Lcd.drawLine(1, 15+(127-setpoint), 4, 15+(127-setpoint), 0x7bef); // draw line from X, Y to x, y // old line
  // M5.Lcd.drawLine(26, 15+(127-setpoint), 29, 15+(127-setpoint), 0x7bef); // draw line from X, Y to x, y // old line
  M5.Lcd.drawLine(1, 15+(pressure_out-setpoint), 4, 15+(pressure_out-setpoint), 0x7bef); // draw line from X, Y to x, y
  M5.Lcd.drawLine(x_calibration, 15+(pressure_out-setpoint), x_calibration_end, 15+(pressure_out-setpoint), 0x7bef); // draw line from X, Y to x, y  
}

void setLED(bool isON)
{
  digitalWrite (ledPin, !isON); // set the LED
}

void progressBar(int value)
{
  // Value is expected to be in range 0-127
  for (int i = 0; i <= value; i++) {  //draw bar
    // M5.Lcd.fillRect(8, 142-i, 15, 1, rainbow(i)); // X, Y, width, height from rectangle to fill, if Height = 1 its like a line. // old line
    M5.Lcd.fillRect(8, x_progressbar-i, x_progressbar_width, 1, rainbow(i)); // X, Y, width, height from rectangle to fill, if Height = 1 its like a line.
  }
 
 // for (int i = value+1; i <= 128; i++) {  //clear old stuff old line
  for (int i = value+1; i <= (pressure_out+1); i++) {  //clear old stuff
    // M5.Lcd.fillRect(8, 142-i, 15, 1, BLACK); // clean rectangle with black // old line
    M5.Lcd.fillRect(8, x_progressbar-i, x_progressbar_width, 1, BLACK); // clean rectangle with black
  }
}

unsigned int rainbow(int value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to red = blue
  //int value = random (128);
  byte red = 0; // Red is the top 5 bits of a 16 bit colour value 1111 1000 0000 0000 63 level of red
  byte green = 0;// Green is the middle 6 bits 0000 0111 1110 0000 63 level of green
  byte blue = 0; // Blue is the bottom 5 bits 0000 0000 0001 1111 31 level of blue

  byte quadrant = value / adjust_color;
  int valb = map(value, 0, pressure_out, 0, 127);
  if (quadrant == 0) {
    blue = 31;
    green = 2 * (valb % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - 1 - (valb % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = valb % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (valb % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

// Old version
/*unsigned int rainbow(int value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to red = blue
  //int value = random (128);
  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;
    int valb = value;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}*/
