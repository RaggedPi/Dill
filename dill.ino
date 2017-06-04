/*
  RaggedPi Project
  Arduino "Dill" - Refridgeration thermostat controller
  Written by david durost <david.durost@gmail.com>
*/
/* Includes */
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Relay.h>
#include <SPI.h>
#include <SoftwareSerial.h>

/* Constants */
#define SDA 5               // sda`
#define SDL 6               // sdl
#define CS 10               // cs
#define LED 13              // pin
#define DC 3                // dc
#define RESET 7             // pin
#define DHTPIN 4            // analog pin
#define SLEEP_TIME 9000    // ms
#define MAXTEMP 46.00       // *F
#define MINTEMP 40.00       // *F
#define TEMPOVERUNDER 6.00  // +/- *F

/* Enums */
enum Modes {
    MONITOR_MODE,
    SLEEP_MODE,
    SETTEMP_MODE
};

/* Objects */
Relay fridge(RELAY1);
OneWire oneWire(DHTPIN);
DallasTemperature sensors(&oneWire);

/* Variables */
unsigned long sleep = 0;
float tempC = 0.00;
float tempF = 0.00;
float tempSetting = 42.00;
bool coolRun = false;
bool boot = true;
Modes state = MONITOR_MODE;

/**
 * Monitor mode
 */
void monitorMode() {
    Serial.println("Monitoring...");

    readTemps();
    displayTemps(); 

    if (tempF > tempSetting) {
        fridge.on();

        Serial.println("Cooling.");

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
    long sleeptime = millis() - sleep;
    fridge.off();
    Serial.println("Sleeping...");
    Serial.print("Sleep time: ");
    Serial.println(sleeptime);

    readTemps();
    displayTemps();

    if (sleeptime > SLEEP_TIME) {
        coolRun = false;
        state = MONITOR_MODE;
    }
}

/**
 * Read temperature
 */
void readTemps() {
    //sensors.requestTemperatures();
    //tempC = sensors.getTempCByIndex(0);
    tempC = analogRead(DHTPIN);
    tempF = (tempC * 9.0) / 5.0 + 32.0;
}

/**
 * Display temps
 */
void displayTemps() {
    Serial.print("Temp: ");
    Serial.print(tempC);
    Serial.println("C\t");
    Serial.print("Temp: ");
    Serial.print(tempF);
    Serial.println("F\t");
}

/**
 * Setup
 */
void setup() {
    pinMode(LED, OUTPUT);
    pinMode(CS, OUTPUT);
    pinMode(DHTPIN, INPUT);

    Serial.begin(9600);

    sensors.begin();
    fridge.begin();
    Serial.println("Initializing...");    
}

/**
 * Loop
 */
void loop() {
    delay(10000);

    switch(state) {
        case MONITOR_MODE:
            monitorMode();
            break;
        case SLEEP_MODE:
            sleepMode();
            break;
    }
}