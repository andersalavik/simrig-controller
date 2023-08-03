#include <EEPROM.h>
#include "HX711.h"
#include "Joystick.h"

#define DOUT 2
#define CLK 3

// Enum for the curve type
enum curveType { LINEAR, EXPONENTIAL, LOGARITHMIC };

// Setup HX711 scale object
HX711 handbrakeScale;

// Struct to hold the settings
struct Settings {
  curveType handbrakeCurve;
  float minRawHandbrake;
  float maxRawHandbrake;
  float curveFactor;  // Additional parameter to adjust curve behavior
};

// Define default settings
Settings defaultSettings = { EXPONENTIAL, 10000.00, 2095588, 2.0 };

// Variable to hold current settings
Settings currentSettings;

// Joystick object
Joystick_ myJoystick(JOYSTICK_DEFAULT_REPORT_ID + 7, JOYSTICK_TYPE_MULTI_AXIS, 0, 0, false, false, false, false, false, false, false, true, false, true, false);

bool setupMode = false; // Added setup mode flag

void setup() {
  // Load settings from EEPROM
  EEPROM.get(0, currentSettings);

  // Check if settings are valid
  if (currentSettings.handbrakeCurve < LINEAR || currentSettings.handbrakeCurve > LOGARITHMIC || currentSettings.minRawHandbrake < -9000000 || currentSettings.maxRawHandbrake > 90000000 || currentSettings.curveFactor <= 0) {
    // If not valid, use default settings
    currentSettings = defaultSettings;
  }

  // Setup LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // Indicate start of setup

  // Initialize HX711
  handbrakeScale.begin(DOUT, CLK);
  handbrakeScale.set_scale();
  handbrakeScale.tare();  // Tare the scale

  // Initialize joystick
  myJoystick.setBrakeRange(0, 1023);  // Set brake range
  myJoystick.begin(true);  // Start joystick in auto send mode

  digitalWrite(LED_BUILTIN, LOW);  // Indicate end of setup
  
  // Initialize Serial
  Serial.begin(9600);
}

void applyCurve(float& val, curveType curve, float curveFactor) {
  // Shift the range of val to a positive domain
  val = val - currentSettings.minRawHandbrake;

  // Make sure val is in the range 0-700
  val = constrain(val, 0, currentSettings.maxRawHandbrake - currentSettings.minRawHandbrake);
  
  // Apply curve depending on the curve type
  switch(curve) {
    case LINEAR:
      val = val / (currentSettings.maxRawHandbrake - currentSettings.minRawHandbrake);  // Linear curve
      break;
    case EXPONENTIAL:
      val = pow(val / (currentSettings.maxRawHandbrake - currentSettings.minRawHandbrake), curveFactor);  // Exponential curve with custom exponent
      break;
    case LOGARITHMIC:
      // Logarithmic curve with custom base (check to avoid log of 0)
      if (val > 0) // Now val is guaranteed to be non-negative, we only need to check if it's greater than 0
        val = log(val / (currentSettings.maxRawHandbrake - currentSettings.minRawHandbrake)) / log(curveFactor);
      else
        val = 0;
      break;
  }

  // Apply transformation to joystick brake range (0 to 1023)
  val = val * 1023;
}



void loop() {
  // Check for incoming messages
  if (Serial.available() > 0) {
    // Read the command
    char command = Serial.read();

    // Handle the command
    switch(command) {
      case 'c':  // Change curve type
        if (Serial.available() > 0) {
          curveType newCurve = (curveType)Serial.parseInt();
          if (newCurve >= LINEAR && newCurve <= LOGARITHMIC) {
            currentSettings.handbrakeCurve = newCurve;
          }
        }

        break;
      case 'm':  // Change minRawHandbrake
        if (Serial.available() >= sizeof(float)) {
          float newMin = Serial.parseFloat();
          if (newMin >= -900000) {
            currentSettings.minRawHandbrake = newMin;
          }
        }
        break;
      case 't':  // Change maxRawHandbrake
        if (Serial.available() >= sizeof(float)) {
          float newMax = Serial.parseFloat();
          if (newMax <= 9000000) {
            currentSettings.maxRawHandbrake = newMax;
          }
        }
        break;
      case 'f':  // Change curveFactor
        if (Serial.available() > 0) {
          int newFactor = Serial.parseInt();
          if (newFactor >= 0) {
            currentSettings.curveFactor = float(newFactor) / 10.0;
          }
        }
        break;

      case 's':  // Save settings
        EEPROM.put(0, currentSettings);
        break;
      case 'e':  // Toggle setup mode
        setupMode = true;
        break;
      case 'w':  // Toggle setup mode
        setupMode = false;
        break;
      case 'r':  // Read current settings
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

  // Read handbrake value
  float rawHandbrake = handbrakeScale.get_units();
  float rawHandbrakeOriginal = rawHandbrake;  // Store the original raw value

  // Apply curve to the raw reading
  applyCurve(rawHandbrake, currentSettings.handbrakeCurve, currentSettings.curveFactor);

  // Set brake value on joystick
  myJoystick.setBrake(rawHandbrake);

  // Check if setup mode is active
  if (setupMode) {
    // Print raw and processed handbrake value
    Serial.print("Raw Handbrake Value: ");
    Serial.print(rawHandbrakeOriginal);
    Serial.print("   Processed Handbrake Value: ");
    Serial.println(rawHandbrake);
  }
}