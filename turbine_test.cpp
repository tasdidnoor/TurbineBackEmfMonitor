// Turbine voltage test (ESP32-C3 SuperMini)
//
// This only measures and prints. No LEDs, buzzer or screen.
// Goal: see if "still", "normal" and "fast" give clearly different numbers.
//
// Wiring: one motor wire to GPIO 1, the other motor wire to GND.
// (No voltage divider. 80 mV is safe to connect directly.)

#include <Arduino.h>

const int ADC_PIN = 1;

// Measured at startup while the blades are still
float zeroOffset = 0;


// Reads the pin 100 times and returns the average, in millivolts.
float readMillivolts() {
  float total = 0;

  for (int i = 0; i < 100; i++) {
    total = total + analogReadMilliVolts(ADC_PIN);
  }

  return total / 100;
}


void setup() {
  Serial.begin(115200);
  delay(2000);   // gives the serial monitor time to connect

  // Use the most sensitive range (about 0 to 750 mV)
  analogSetPinAttenuation(ADC_PIN, ADC_0db);

  Serial.println("Keep the blades STILL for 3 seconds...");
  delay(3000);

  // Measure what the ADC reads when nothing is happening, 5 times, and average
  float total = 0;
  for (int i = 0; i < 5; i++) {
    total = total + readMillivolts();
  }
  zeroOffset = total / 5;

  Serial.print("Zero offset: ");
  Serial.print(zeroOffset, 1);
  Serial.println(" mV");
  Serial.println("Ready. Start the blades now.");
  Serial.println();
}


void loop() {
  float reading = readMillivolts();
  float adjusted = reading - zeroOffset;

  Serial.print("raw: ");
  Serial.print(reading, 1);
  Serial.print(" mV    adjusted: ");
  Serial.print(adjusted, 1);
  Serial.println(" mV");

  delay(200);
}
