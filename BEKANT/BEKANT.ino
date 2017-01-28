/*
  Sketch to control driving an IKEA electrical desk (BEKANT) up and down.
  Author: Robert.Schneider@aramar.de

  During development, I used an EA DOGM 162 display to visualize certain
  values of the control logic. This was mainly to free me from wiring my
  Arduino to the computer while testing with the desk.

  We assume  the following pins are connected:
    LCD SI pin to digital pin 2
    LCD CLK pin to digital pin 3
    LCD RS pin to digital pin 4
    LCD CSB pin to digital pin 5
    LCD RESET pin is not used
    LCD Backlight pin is not used

  To measure the current height of the desk, an ultrasonic sensor (HC-SR04)
  is used which is mounted below the desktop.

  We assume the following pins are connected:
    HC-SR04 echo to digital pin 6
    HC-SR04 trigger to digital pin 7

*/

// include the library code:
#include <DogLcd.h>
#include <NewPing.h>

// Watchdog variables, watchdog will blink the internal LED periodically.
boolean watchdogEnabled = true;
int watchdogDelay = 250; // ms

// LCD variables
// initialize the library with the numbers of the interface pins
// DogLcd(int SI, int CLK, int RS, int CSB, int RESET=-1, int backLight=-1);
DogLcd lcd(4, 5, 6, 7);

// Define custom characters that look like arrows
byte arrowDownData[] = {0x04, 0x04, 0x04, 0x04, 0x15, 0x0E, 0x04, 0x00};   // Pattern for down arrow
byte arrowAutoDownData[] = {0x04, 0x15, 0x0e, 0x04, 0x15, 0x0e, 0x04, 0x00};
byte arrowUpData[] = {0x00, 0x04, 0x0E, 0x15, 0x04, 0x04, 0x04, 0x04};     // Pattern for up arrow
byte arrowAutoUpData[] = {0x00, 0x04, 0x0e, 0x15, 0x04, 0x0e, 0x15, 0x04};
byte stopData[] = {0x00, 0x0e, 0x1f, 0x1f, 0x1f, 0x0e, 0x00, 0x00};
int arrowDownChar = 1;
int arrowUpChar = 2;

// Ultrasonice sensor variables
#define TRIGGER_PIN  8    // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     9    // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 140  // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
long usDistanceInCm = 0;    // Computed distance in cm.

// Buttons
#define BUTTON_UP_PIN 2
#define BUTTON_DN_PIN 3
#define BUTTON_MEM_PIN 10
volatile int buttonUpState = LOW;
volatile int buttonDnState = LOW;
volatile int buttonMemState = LOW;

// Relais
#define RELAIS_UP_PIN 11
#define RELAIS_DN_PIN 12
int relaisUpState = HIGH;
int relaisDnState = HIGH;

void setup() {
  setupButtons();
  setupWatchdog();
  setupLCD();
  setupRelais();
}

void loop() {
  loopWatchdog();
  loopButtons();
  loopLCD();
  loopUltrasonic();
}

void setupButtons() {
  // initialize the pushbutton pins as an inputs:
  pinMode(BUTTON_UP_PIN, INPUT);
  digitalWrite(BUTTON_UP_PIN, HIGH);
  pinMode(BUTTON_DN_PIN, INPUT);
  digitalWrite(BUTTON_DN_PIN, HIGH);
  pinMode(BUTTON_MEM_PIN, INPUT);
  digitalWrite(BUTTON_MEM_PIN, HIGH);
  // attach the interrupts for up and down
  attachInterrupt(digitalPinToInterrupt(BUTTON_UP_PIN), isrButtonUp, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON_DN_PIN), isrButtonDown, CHANGE);
}

void setupWatchdog() {
  if (watchdogEnabled) {
    pinMode(LED_BUILTIN, OUTPUT);
  }
}

void setupLCD() {
  // set up the LCD type and the contrast setting for the display
  lcd.begin(DOG_LCD_M162, 0x3f);
  lcd.noCursor();
  lcd.noBlink();
  lcd.createChar(arrowDownChar, arrowDownData);
  lcd.createChar(arrowUpChar, stopData);
  // Print a message to the LCD.
  lcd.home();
  lcd.print("Hello, world!");
}

void setupRelais() {
  pinMode(RELAIS_UP_PIN, OUTPUT);
  digitalWrite(RELAIS_UP_PIN, HIGH);
  pinMode(RELAIS_DN_PIN, OUTPUT);
  digitalWrite(RELAIS_DN_PIN, HIGH);
}

void loopWatchdog() {
  if (watchdogEnabled) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(watchdogDelay);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(watchdogDelay);
  }
}

void loopButtons() {
  // read the state of the pushbutton value:
  int newbuttonMemState = digitalRead(BUTTON_MEM_PIN);
  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (newbuttonMemState != buttonMemState) {
    lcd.setCursor(15, 0);
    if (newbuttonMemState == LOW) {
      lcd.write(arrowUpChar);
    } else {
      lcd.print(" ");
    }
    buttonMemState = newbuttonMemState;
  }
}

void loopLCD() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);
}

void loopUltrasonic() {
  lcd.print(" - ");
  usDistanceInCm = sonar.convert_cm(sonar.ping_median(5));
  if (usDistanceInCm >= 500 || usDistanceInCm <= 0) // Print an error if value doesn't make sense
  {
    lcd.print("? cm  "); // ? means, we don't know the real value
  }
  else
  {
    lcd.print(usDistanceInCm); // print value of cm
    lcd.print(" cm  "); // print unit
  }
}

void isrButtonUp() {
  buttonUpState = digitalRead(BUTTON_UP_PIN);
  digitalWrite(RELAIS_UP_PIN, buttonUpState);
}

void isrButtonDown() {
  buttonUpState = digitalRead(BUTTON_DN_PIN);
  digitalWrite(RELAIS_DN_PIN, buttonUpState);
}

