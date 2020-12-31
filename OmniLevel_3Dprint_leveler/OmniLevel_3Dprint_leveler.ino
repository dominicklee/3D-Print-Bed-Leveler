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

#include <M5StickC.h>
#include <EEPROM.h>
#include "OneButton.h"
#include "rotate-left.h"
#include "rotate-right.h"
#include "thumbs-up.h"

// Icons converted at: 
// http://rinkydinkelectronics.com/_t_doimageconverter565.php

#define EEPROM_SIZE 1
int ledPin = G10;
int pressure = 0;   //the var that goes between 0-127 from FSR reading
int setpoint = 64;  //to be updated by EEPROM
float errorThreshold = 6.0; //printbed is considered leveled if error goes below this (ideally 5.0-8.0)

//Object initialization
OneButton btnA(G37, true);
OneButton btnB(G39, true);

void setup() {
  M5.begin();
  EEPROM.begin(EEPROM_SIZE);  //Initialize EEPROM
  Serial.begin(115200);       //Initialize Serial
  pinMode(ledPin, OUTPUT);    //Set up LED
  digitalWrite (ledPin, HIGH); // turn off the LED

  M5.Lcd.setRotation(0);
  M5.Lcd.fillScreen(TFT_BLACK);

  btnA.attachClick(btnAClick);  //BtnA handle
  btnA.setDebounceTicks(40);
  btnB.attachClick(btnBClick);  //BtnB handle
  btnB.setDebounceTicks(25);
  
  M5.Lcd.drawRect(5, 12, 21, 133, 0x7bef);  //show frame for progressbar
  getCalibration(); //show calibration mark

  //Show OFF instructions
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(42, 75);
  M5.Lcd.printf("Off ->");

  //Show Calibrate instructions
  M5.Lcd.setTextColor(RED);
  M5.Lcd.setCursor(15, 150);
  M5.Lcd.printf("Calibrate");

  // Swap the colour byte order when rendering
  M5.Lcd.setSwapBytes(true);
}

void loop() {
  //poll for button press
  btnA.tick();
  btnB.tick();
  
  pressure = map(analogRead(G36), 0, 4095, 0, 127); //get reading
  Serial.println(pressure);
  
  progressBar(pressure);  //show reading on progressbar
  
  if (getPercentError(pressure, setpoint) < errorThreshold) {
    setLED(true);
    M5.Lcd.pushImage(40, 15, 32, 32, thumbs_up);  //Draw icon
  } else {
    setLED(false);
    if (pressure > setpoint) {
      M5.Lcd.pushImage(40, 15, 32, 32, rotate_left);  //Draw icon
    } else {
      M5.Lcd.pushImage(40, 15, 32, 32, rotate_right);  //Draw icon
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
  M5.Lcd.drawLine(1, 15+(127-setpoint), 4, 15+(127-setpoint), BLACK);
  M5.Lcd.drawLine(26, 15+(127-setpoint), 29, 15+(127-setpoint), BLACK);
  // set new line
  setpoint = value; //set global
  M5.Lcd.drawLine(1, 15+(127-setpoint), 4, 15+(127-setpoint), 0x7bef);
  M5.Lcd.drawLine(26, 15+(127-setpoint), 29, 15+(127-setpoint), 0x7bef);
}

void getCalibration()
{
  setpoint = EEPROM.read(0);  // retrieve calibration in EEPROM
  // set new line
  M5.Lcd.drawLine(1, 15+(127-setpoint), 4, 15+(127-setpoint), 0x7bef);
  M5.Lcd.drawLine(26, 15+(127-setpoint), 29, 15+(127-setpoint), 0x7bef);
}

void setLED(bool isON)
{
  digitalWrite (ledPin, !isON); // set the LED
}

void progressBar(int value)
{
  // Value is expected to be in range 0-127
  for (int i = 0; i <= value; i++) {  //draw bar
    M5.Lcd.fillRect(8, 142-i, 15, 1, rainbow(i));
  }
  for (int i = value+1; i <= 128; i++) {  //clear old stuff
    M5.Lcd.fillRect(8, 142-i, 15, 1, BLACK);
  }
}

unsigned int rainbow(int value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to red = blue
  //int value = random (128);
  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;

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
}
