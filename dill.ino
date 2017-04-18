/*
  RaggedPi Project
  Arduino "Dill" - Thermometer controller
  Written by david durost <david.durost@gmail.com>
*/
#include <Wire.h>
#include <OneWire.h>
#include <OneButton.h>
#include <DallasTemperature.h>
#include <Relay.h>
//#include <DHT.h>

// Constants
#define SDA 5
#define SDL 6
#define CS 10
#define LED 13
#define DHTPIN 4
#define FRIDGEPIN 4 // digital pin
#define UPPIN 8
#define DNPIN 9
#define SETPIN 3
#define GNLEDPIN 6 // digital pin
#define RDLEDPIN 5 // digital pin
#define SLEEP_TIME 900000
#define MAXTEMP 40.00
#define MINTEMP 32.00

// Enums
enum Modes {
    MONITOR_MODE,
    SLEEP_MODE,
    SETTEMP_MODE
};

// Objects
Relay fridge(FRIDGEPIN);
OneButton upBtn(UPPIN, true);
OneButton dnBtn(DNPIN, true);
OneButton setBtn(SETPIN, true);
OneWire oneWire(DHTPIN);
DallasTemperature sensors(&oneWire);
// DHT dht(DHTPIN, DHT11);

// Variables
unsigned long sleep = 0;
float tempSetting = 35.00;
float tempC = 0;
float tempF = 0;
bool coolRun = false;
bool boot = true;
Modes state = MONITOR_MODE;

/**
 * Increase Temperature
 */
void increaseTemp() {
    tempSetting += 1.00;

    if (tempSetting > MAXTEMP) {
        tempSetting = MAXTEMP;
    }
}

/**
 * Decrease temperature
 */
void decreaseTemp() {
    tempSetting -= 1.00;

    if (tempSetting <= MINTEMP) {
        tempSetting = MINTEMP;
    }
}

/**
 * Monitor mode
 */
void monitorMode() {
    // Set buttons
    upBtn.attachClick(setTempMode);
    upBtn.tick();
    dnBtn.attachClick(setTempMode);
    dnBtn.tick();
    setBtn.attachClick(setTempMode);
    setBtn.tick();

    // Toggle relay
    if (tempF <= tempSetting) {
        fridge.on();        
    }

    readTemps();
    displayTemps(); 

    if (tempF > tempSetting) {
        coolRun = true;
    }

    if (coolRun == true && tempF <= tempSetting) {
        sleep = millis();
        state = SLEEP_MODE;
    }
}

/**
 * Sleep mode
 */
void sleepMode() {
    upBtn.attachClick(setTempMode);
    upBtn.tick();
    dnBtn.attachClick(setTempMode);
    dnBtn.tick();
    setBtn.attachClick(setTempMode);
    setBtn.tick();

    fridge.off();
    readTemps();
    displayTemps();

    if (millis() - sleep > SLEEP_TIME) {
        coolRun = false;
        state = MONITOR_MODE;
    }
}

/**
 * Set temperature mode
 */
void setTempMode() {
    upBtn.attachClick(increaseTemp);
    upBtn.tick();
    dnBtn.attachClick(decreaseTemp);
    dnBtn.tick();
}

/**
 * Read temperature
 */
void readTemps() {
    sensors.requestTemperatures();
    tempC = sensors.getTempCByIndex(0);
    tempF = (tempC * 9.0) / 5.0 + 32.0;
}

void displayTemps() {
    // output to display screen
}

/**
 * Setup
 */
void setup() {
    pinMode(LED, OUTPUT);
    pinMode(CS, OUTPUT);
    pinMode(GNLEDPIN, OUTPUT);
    pinMode(RDLEDPIN, OUTPUT);

//    dht.begin();
    sensors.begin();
    fridge.begin();
}

/**
 * Loop
 */
void loop() {
    switch(state) {
        case MONITOR_MODE:
            monitorMode();
            break;
        case SLEEP_MODE:
            sleepMode();
            break;
        case SETTEMP_MODE:
            setTempMode();
            break;
    }
}