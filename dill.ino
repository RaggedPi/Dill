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
#include <LiquidCrystal.h>

/* Constants */
#define SDA 5               // sda`
#define SDL 6               // sdl
#define CS 10               // cs
#define LED 13              // pin
#define DC 3                // dc
#define RESET 7             // pin
#define DHTPIN 4            // analog pin
#define SLEEP_TIME 9000     // ms
#define MAXTEMP 46.00       // *F
#define MINTEMP 40.00       // *F
#define TEMPOVERUNDER 6.00  // +/- *F
#define BTNPIN 0            // analog pin
#define RELAYPIN 11         // digital pin
#define UP_BTN 2
#define DOWN_BTN 3
#define LEFT_BTN 4
#define RIGHT_BTN 1
#define SELECT_BTN 5

/* Enums */
enum Modes {
    MONITOR_MODE,
    SLEEP_MODE,
    SETTEMP_MODE,
    OVERRIDE_MODE
};

/* Objects */
Relay fridge(RELAY1);
OneWire oneWire(DHTPIN);
DallasTemperature sensors(&oneWire);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

/* Variables */
unsigned long sleep = 0;
float tempC = 0.00;
float tempF = 0.00;
float tempSetting = 42.00;
float newTempSetting = tempSetting;
bool coolRun = false;
bool boot = true;
Modes state = MONITOR_MODE;
bool override = false;


/**
 * clear line on lcd
 * @param  int line     default= 0
 * @param  int startChar default= 0
 * @param  int endChar  default=16
 */
void clearLine(int line=0, int startChar=0, int endChar=16) {
    lcd.setCursor(startChar, line);
    if (0 != startChar && 16 > startChar) startChar -= 16;
    else    startChar = endChar;
    for(int x=0; x < endChar; x++)   lcd.write(' ');  
}

/**
 * Change temperature setting
 * @param  int chg]
 */
void changeTempSetting(int chg) {
    newTempSetting += chg;
    lcd.clear();
    lcd.print("Set Temp: ");
    lcd.print((int)newTempSetting);
    lcd.print("F");
    lcd.setCursor(0,1);
    lcd.autoscroll();
    lcd.print("Push SELECT to save.");
} 

/**
 * Save temperature setting
 */
void saveTempSetting() {
    tempSetting = newTempSetting;
    lcd.clear();
    lcd.print("Temp saved.");
    delay(3000);
    state = MONITOR_MODE;
}

/**
 * Button push actions
 */
void btnAction() {
    #ifdef WAKE_LCD_ON_BTN
        if (!lcd_awake) {
            ~lcd_awake;
            return;
        }
    #endif

    switch (readLCDBtns()) {
        case UP_BTN:
            changeTempSetting(1);
            break;
        case DOWN_BTN:
            changeTempSetting(-1);
            break;
        case LEFT_BTN:
            break;
        case RIGHT_BTN:
            break;
        case SELECT_BTN:
            saveTempSetting();
            break;
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
 * Draw temp on lcd
 */
void drawTemp() {
    clearLine(1, 0, 3);
    lcd.setCursor(0, 1);
    lcd.print((int)tempF);
    lcd.print("F");
}

/**
 * Read LCD buttons
 * @return int
 */
int readLCDBtns() {
    uint8_t btn = analogRead(BTNPIN);

    lcd.clear();
    lcd.print(btn);
    switch(btn) {
        case 50:
            return RIGHT_BTN;
            break;
        case 195:
            return UP_BTN;
            break;
        case 380:
            return DOWN_BTN;
            break;
        case 650:
            return LEFT_BTN;
            break;
        case 790:
            return SELECT_BTN;
            break;
    }

    return 0;
}

/**
 * Draw action message on lcd
 */
void drawAction() {
    char* msg;

    clearLine();
    switch(state) {
        case MONITOR_MODE:
            msg = "Monitoring...";
            break;
        case SLEEP_MODE:
            msg = "Sleeping...";
            break;
        case OVERRIDE_MODE:
            msg = "Override Active";
            break;
        case SETTEMP_MODE:
            break;
    }
    clearLine();
    lcd.home();
    lcd.print(msg);
}

/**
 * Draw cooling message on lcd
 * @param  bool on default: true
 */
void drawCooling(bool on=true) {
    lcd.setCursor(6, 1);
    if (on) lcd.print("[Cooling]");
    else    lcd.print("         ");
}

/**
 * Monitor mode
 */
void monitorMode() {
    Serial.println("Monitoring...");
    
    readTemps();
    drawAction();
    drawTemp();
    displayTemps(); 

    if (tempF > tempSetting) {
        fridge.on();

        Serial.println("Cooling.");
        drawCooling();

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
    drawAction();
    drawTemp();
    drawCooling(false);
    displayTemps();

    if (sleeptime > SLEEP_TIME) {
        coolRun = false;
        state = MONITOR_MODE;
    }
}

/**
 * Display temps to serial
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
    for(int i=4;i<10;i++){
        pinMode(i,OUTPUT);
    }

    Serial.begin(9600);

    sensors.begin();
    fridge.begin();
    Serial.println("Initializing...");    
    lcd.begin(16, 2);
    digitalWrite(CS, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Initializing...");
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
           
    btnAction();
}