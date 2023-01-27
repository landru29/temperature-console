
#include "temperature.h"
#include <Wire.h>
#include "FS.h"

#include <SPI.h>
#include <TFT_eSPI.h>      // Hardware-specific library
#define offset 80

#define LABEL1_FONT &FreeSansOblique12pt7b // Key label font 1

// #define DEBUG

#define CALIBRATION_FILE "/TouchCalData"
#define REPEAT_CAL false

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

TFT_eSPI_Button chronoStart;
TFT_eSPI_Button chronoStop;
TFT_eSPI_Button chronoReset;

bool chronoRunning;
int chronoValue;

hw_timer_t *heartBeat = NULL;
bool latch;


void IRAM_ATTR onTimer(){
  if (chronoRunning) chronoValue++;

  latch = true;
}

void setup() {
  latch = false;
  chronoValue = 0;
  chronoRunning = false;

  char startLabel[] = "start";
  char stopLabel[] = "stop";
  char resetLabel[] = "reset";

  Wire.begin(); 

#ifdef DEBUG
  Serial.begin(9600);
#endif

  tft.init();

  tft.setRotation(0);

  //touch_set_calibration();
  touch_calibrate();

  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  tft.setTextFont(5);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("Brassage", 160, 15, 4);
  tft.drawLine(0, 50, 320, 50, TFT_WHITE);

  tft.setFreeFont(LABEL1_FONT);

  chronoStart.initButton(&tft, 80,  430, 65, 40, TFT_WHITE, TFT_BLUE, TFT_WHITE, startLabel, 1);
  chronoStop.initButton( &tft, 160, 430, 65, 40, TFT_WHITE, TFT_BLUE, TFT_WHITE, stopLabel,  1);
  chronoReset.initButton(&tft, 240, 430, 65, 40, TFT_WHITE, TFT_BLUE, TFT_WHITE, resetLabel, 1);
  chronoStart.drawButton();
  chronoStop.drawButton();
  chronoReset.drawButton();

  Temperature::searchSensors();

  heartBeat = timerBegin(0, 80, true);
  timerAttachInterrupt(heartBeat, &onTimer, true);
  timerAlarmWrite(heartBeat, 1000000, true);
  timerAlarmEnable(heartBeat); //Just Enable
}

void loop() {
  processButton();

  displayChrono();

  displayTemperature();
}

void chooseColor(float celcius) {
  tft.setTextColor(TFT_BLACK, TFT_RED);
  if (celcius<75) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
  }

  if (celcius<70) {
    tft.setTextColor(TFT_PURPLE, TFT_BLACK);
  }

  if (celcius<65) {
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
  }

  if (celcius<62) {
    tft.setTextColor(TFT_PINK, TFT_BLACK);
  }

  if (celcius<50) {
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
  }

  if (celcius<43) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
  }
}

void touch_set_calibration() {
  uint16_t calData[5];

  calData[0] = 400;
  calData[1] = 3200;
  calData[2] = 300;
  calData[3] = 3200;
  calData[4] = 0;

  tft.setTouch(calData);
}

void processButton() {
  tft.setFreeFont(LABEL1_FONT);

  uint16_t t_x = 0, t_y = 0; // To store the touch coordinates

  // Pressed will be set true is there is a valid touch on the screen
  bool pressed = tft.getTouch(&t_x, &t_y);

  if (pressed && chronoStart.contains(t_x, t_y)) {
    chronoRunning = true;
    chronoStart.press(true);
    #ifdef DEBUG
        Serial.println("start");
    #endif
  } else {
    chronoStart.press(false);
  }

  if (pressed && chronoStop.contains(t_x, t_y)) {
    chronoRunning = false;
    chronoStop.press(true);
    #ifdef DEBUG
        Serial.println("stop");
    #endif
  } else {
    chronoStop.press(false);
  }

  if (pressed && chronoReset.contains(t_x, t_y)) {
    chronoValue = 0;
    chronoRunning = false;
    chronoReset.press(true);
    #ifdef DEBUG
        Serial.println("reset");
    #endif
  } else {
    chronoReset.press(false);
  }

  if (chronoStart.justReleased()) chronoStart.drawButton();
  if (chronoStop.justReleased()) chronoStop.drawButton();
  if (chronoReset.justReleased()) chronoReset.drawButton();

  if (chronoStart.justPressed()) chronoStart.drawButton(true);
  if (chronoStop.justPressed()) chronoStop.drawButton(true);
  if (chronoReset.justPressed()) chronoReset.drawButton(true);

   //delay(10); // UI debouncing
}

void displayTemperature() {
  if (!latch) {
    return;
  }

  latch = false;

  if (Temperature::count) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextFont(5);

    for (int i = 0; i<Temperature::count; i++) {
      float celcius = Temperature::sensor[i]->value();

      chooseColor(celcius);
      
      String out = String(celcius, 1);
      if (i%2) {
        tft.drawCentreString(out, 80, offset + 10+60*(i/2), 6);
      } else {
        tft.drawCentreString(out, 240, offset + 10+60*(i/2), 6);
      }

      #ifdef DEBUG
        Serial.println(out);
      #endif
    }
    
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawCentreString("no sensor", 160, offset + 10, 4);
    tft.drawCentreString("found", 160, offset + 40, 4);
    #ifdef DEBUG
      Serial.println("no sensor found");
    #endif
  }
}

void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

void displayChrono() {
  int hour = chronoValue / 3600;
  int minutes = (chronoValue - 3600 * hour) / 60;
  int seconds = chronoValue % 60;

  char buffer[9];
  sprintf(buffer, "%02d:%02d:%02d", hour, minutes, seconds);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString(buffer, 160, 350, 6);
}

