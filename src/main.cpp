#include <EEPROM.h>
#include "HX711.h"
#include "Joystick.h"

#define DOUT 2
#define CLK 3

#define FILTER_SIZE 10  // Size of the filter window. Adjust this as necessary

float filterBuffer[FILTER_SIZE];  // Buffer to hold past readings
int filterIndex = 0;  // Index for the current position in the buffer


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
Settings defaultSettings = { LINEAR, -5200, 50000, 2.0 };

// Variable to hold current settings
Settings currentSettings;

// Joystick object
Joystick_ myJoystick(JOYSTICK_DEFAULT_REPORT_ID + 7, JOYSTICK_TYPE_MULTI_AXIS, 0, 0, false, false, false, false, false, false, false, true, false, true, false);

bool setupMode = false; // Added setup mode flag

void setup() {
  // Load settings from EEPROM
  EEPROM.get(0, currentSettings);

  // Initialize filter buffer with zeros
  for (int i = 0; i < FILTER_SIZE; i++) {
    filterBuffer[i] = 0;
  }
  
  // Check if settings are valid
  if (currentSettings.handbrakeCurve < LINEAR || currentSettings.handbrakeCurve > LOGARITHMIC || currentSettings.minRawHandbrake < 0 || currentSettings.maxRawHandbrake > 900000 || currentSettings.curveFactor <= 0) {
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

// Function to apply a curve to a value
void applyCurve(float& val, curveType curve, float curveFactor) {
  float mHandbrake = 1023 / (currentSettings.maxRawHandbrake - currentSettings.minRawHandbrake);
  float cHandbrake = -mHandbrake * currentSettings.minRawHandbrake;
  
  // Apply curve depending on the curve type
  switch(curve) {
    case LINEAR:
      val = (mHandbrake * val) + cHandbrake;  // Linear curve
      break;
    case EXPONENTIAL:
      val = mHandbrake * pow(val, curveFactor) + cHandbrake;  // Exponential curve with custom exponent
      break;
    case LOGARITHMIC:
      // Logarithmic curve with custom base (check to avoid log of 0)
      if (val > 0)
        val = mHandbrake * log(val) / log(curveFactor) + cHandbrake;
      else
        val = 0;
      break;
  }
}

void loop() {
  // Check for incoming messages
  if (Serial.available() > 0) {
    // Read the command
    char command = Serial.read();

    // Handle the command
    switch(command) {
      case 'c':  // Change curve type
        if (Serial.available() >= sizeof(int)) {
          curveType newCurve = (curveType)Serial.parseInt();
          if (newCurve >= LINEAR && newCurve <= LOGARITHMIC) {
            currentSettings.handbrakeCurve = newCurve;
          }
        }
        break;
      case 'm':  // Change minRawHandbrake
        if (Serial.available() >= sizeof(float)) {
          float newMin = Serial.parseFloat();
          if (newMin >= 0) {
            currentSettings.minRawHandbrake = newMin;
          }
        }
        break;
      case 'M':  // Change maxRawHandbrake
        if (Serial.available() >= sizeof(float)) {
          float newMax = Serial.parseFloat();
          if (newMax <= 900000) {
            currentSettings.maxRawHandbrake = newMax;
          }
        }
        break;
      case 'f':  // Change curveFactor
        if (Serial.available() >= sizeof(float)) {
          float newFactor = Serial.parseFloat();
          if (newFactor > 0) {
            currentSettings.curveFactor = newFactor;
          }
        }
        break;
      case 's':  // Save settings
        EEPROM.put(0, currentSettings);
        break;
      case 'e':  // Toggle setup mode
        setupMode = !setupMode;
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
        Serial.println(currentSettings.curveFactor);
        break;
      
    }
  }

  // Read handbrake value
  float rawHandbrake = handbrakeScale.get_units();

  // Add reading to filter buffer
  filterBuffer[filterIndex] = rawHandbrake;
  filterIndex = (filterIndex + 1) % FILTER_SIZE;  // Increment index and wrap around

  // Calculate average of past readings
  float avgHandbrake = 0;
  for (int i = 0; i < FILTER_SIZE; i++) {
    avgHandbrake += filterBuffer[i];
  }
  avgHandbrake /= FILTER_SIZE;

  // Apply curve to the average reading
  applyCurve(avgHandbrake, currentSettings.handbrakeCurve, currentSettings.curveFactor);

  // Set brake value on joystick
  myJoystick.setBrake(avgHandbrake);

  // Check if setup mode is active
  if (setupMode) {
    // Print raw and processed handbrake value
    Serial.print("Raw Handbrake Value: ");
    Serial.print(avgHandbrake);  // Use the averaged value here
    Serial.print("   Processed Handbrake Value: ");
    Serial.println(rawHandbrake);
  }
}
