#include <EEPROM.h>
#include "HX711.h"
#include "Joystick.h"

// These are pin assignments for various components
#define DOUT 2
#define CLK 3
#define GUP 7
#define GDOWN 8

enum curveType { LINEAR, EXPONENTIAL, LOGARITHMIC };

HX711 handbrakeScale; // Object for handling the HX711 scale
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID + 7, JOYSTICK_TYPE_MULTI_AXIS, 2, 0, false, false, true, false, false, false, false, true, false, true, false);

struct Settings {
  curveType handbrakeCurve;
  float minRawHandbrake;
  float maxRawHandbrake;
  float curveFactor;
};

// Default settings for the handbrake processing
Settings defaultSettings = { EXPONENTIAL, 10000.00, 2095588, 2.0 };
Settings currentSettings;
bool setupMode = false;
int lastGUpState = 0;
int lastGDownState = 0;

void setup() {
  // Retrieve settings from EEPROM or set defaults if values in EEPROM are out of range
  EEPROM.get(0, currentSettings);
  if (currentSettings.handbrakeCurve < LINEAR || currentSettings.handbrakeCurve > LOGARITHMIC || currentSettings.minRawHandbrake < -9000000 || currentSettings.maxRawHandbrake > 90000000 || currentSettings.curveFactor <= 0) {
    currentSettings = defaultSettings;
  }
  
  // LED setup
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); 
  
  // Setup of pins for Up and Down buttons
  pinMode(GUP, INPUT_PULLUP);
  pinMode(GDOWN, INPUT_PULLUP);
  
  // Handbrake scale setup
  handbrakeScale.begin(DOUT, CLK);
  handbrakeScale.set_scale();
  handbrakeScale.tare();  
  
  // Joystick setup
  Joystick.setZAxisRange(0, 1023);  
  Joystick.setButton(0,0);
  Joystick.setButton(1,0);
  Joystick.begin(true); 
  
  digitalWrite(LED_BUILTIN, LOW);  
  
  // Serial communication setup
  Serial.begin(9600);
}


void applyCurve(float& val, curveType curve, float curveFactor) {
  val = val - currentSettings.minRawHandbrake;
  val = constrain(val, 0, currentSettings.maxRawHandbrake - currentSettings.minRawHandbrake);
  switch(curve) {
    case LINEAR:
      val /= (currentSettings.maxRawHandbrake - currentSettings.minRawHandbrake);
      break;
    case EXPONENTIAL:
      val = pow(val / (currentSettings.maxRawHandbrake - currentSettings.minRawHandbrake), curveFactor);
      break;
    case LOGARITHMIC:
      if (val > 0) 
        val = log(val / (currentSettings.maxRawHandbrake - currentSettings.minRawHandbrake)) / log(curveFactor);
      else
        val = 0;
      break;
  }
  val *= 1023;
}

void handleButton(int& lastState, int pin, int button){
  int currentState = !digitalRead(pin);
  if (currentState != lastState){
    Joystick.setButton(button, currentState);
    lastState = currentState;
  }
}

void loop() {
  while (Serial.available() > 0) {
    char command = Serial.read();
    switch(command) {
      case 'c':
        if (Serial.available() > 0) {
          curveType newCurve = (curveType)Serial.parseInt();
          if (newCurve >= LINEAR && newCurve <= LOGARITHMIC) {
            currentSettings.handbrakeCurve = newCurve;
          }
        }
        break;
      case 'm':
          if (Serial.available() >= static_cast<int>(sizeof(float))) {
            float newMin = Serial.parseFloat();
            if (newMin >= -900000) {
              currentSettings.minRawHandbrake = newMin;
            }
          }
          break;
      case 't':
          if (Serial.available() >= static_cast<int>(sizeof(float))) {
            float newMax = Serial.parseFloat();
            if (newMax <= 9000000) {
              currentSettings.maxRawHandbrake = newMax;
            }
          }
          break;

      case 'f':
        if (Serial.available() > 0) {
          int newFactor = Serial.parseInt();
          if (newFactor >= 0) {
            currentSettings.curveFactor = float(newFactor) / 10.0;
          }
        }
        break;
      case 's':
        EEPROM.put(0, currentSettings);
        break;
      case 'e':
        setupMode = true;
        break;
      case 'w':
        setupMode = false;
        break;
      case 'r':
        Serial.print("Curve type: ");
        switch(currentSettings.handbrakeCurve) {
          case LINEAR:
            Serial.println("LINEAR");
            break;
          case EXPONENTIAL:
            Serial.println("EXPONENTIAL");
            break;
          case LOGARITHMIC:
            Serial.println("LOGARITHMIC");
            break;
        }
        Serial.print("Min raw handbrake: ");
        Serial.println(currentSettings.minRawHandbrake);
        Serial.print("Max raw handbrake: ");
        Serial.println(currentSettings.maxRawHandbrake);
        Serial.print("Curve factor: ");
        Serial.println(currentSettings.curveFactor*10);
        Serial.print(currentSettings.handbrakeCurve);
        break;
    }
  }

  float rawHandbrake = handbrakeScale.get_units();
  float processedHandbrake = rawHandbrake;  // Save the original value
  applyCurve(processedHandbrake, currentSettings.handbrakeCurve, currentSettings.curveFactor);
  Joystick.setZAxis(processedHandbrake);
  handleButton(lastGUpState, GUP, 0);
  handleButton(lastGDownState, GDOWN, 1);

  if (setupMode) {
    Serial.print("Raw Handbrake Value: ");
    Serial.print(rawHandbrake);  // Print the original, unprocessed value
    Serial.print("   Processed Handbrake Value: ");
    Serial.println(processedHandbrake);  // Print the processed value
  }
}
