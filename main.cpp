// Wind turbine monitor - ESP32-C3 SuperMini
// Motor back-EMF on GPIO 0 (other wire to GND), OLED on I2C

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

// pins
const int ADC_PIN       = 0;
const int RED_LED_PIN   = 3;
const int GREEN_LED_PIN = 4;
const int SDA_PIN       = 8;
const int SCL_PIN       = 9;

// thresholds in mV
const float RUN_ON    = 10;
const float RUN_OFF   = 5;
const float WARN_ON   = 60;
const float WARN_OFF  = 45;
const float FAULT_ON  = 100;

// timing in ms
const unsigned long HOLD_MS    = 300;
const unsigned long MEASURE_MS = 50;
const unsigned long SCREEN_MS  = 200;

enum State { STATE_IDLE, STATE_RUNNING, STATE_WARNING, STATE_FAULT };

State state = STATE_IDLE;
State pendingState = STATE_IDLE;
unsigned long pendingSince = 0;

float zeroOffset = 0;   // reading when blades are still
float mv = 0;           // current voltage after offset

unsigned long lastMeasure = 0;
unsigned long lastScreen = 0;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C screen(U8G2_R0, U8X8_PIN_NONE);


void setup() {
  Serial.begin(115200);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  Wire.begin(SDA_PIN, SCL_PIN);
  screen.begin();

  // most sensitive adc range
  analogSetPinAttenuation(ADC_PIN, ADC_0db);

  // message 1
  screen.clearBuffer();
  screen.setFont(u8g2_font_ncenB10_tr);
  screen.setCursor(0, 25);
  screen.print("Keep blades");
  screen.setCursor(0, 45);
  screen.print("still...");
  screen.sendBuffer();
  delay(3000);

  // the zero offset
  float total = 0;
  for (int i = 0; i < 5; i++) {
    total = total + readMillivolts();
  }
  zeroOffset = total / 5;
}


void loop() {
  unsigned long now = millis();

  // main conditionsl
  if (now - lastMeasure >= MEASURE_MS) {
    lastMeasure = now;

    mv = readMillivolts() - zeroOffset;
    if (mv < 0) {
      mv = 0;
    }

    updateState(now);
    setLeds();
  }

  // screen and serial
  if (now - lastScreen >= SCREEN_MS) {
    lastScreen = now;
    showScreen();

    Serial.print(mv, 1);
    Serial.println(" mV");
  }
}


// average of 100 readings, in mV
float readMillivolts() {
  float total = 0;

  for (int i = 0; i < 100; i++) {
    total = total + analogReadMilliVolts(ADC_PIN);
  }

  return total / 100;
}


// state decision
State wantedState() {
  if (state == STATE_FAULT) {
    return STATE_FAULT;   // stays until reset
  }

  if (mv >= FAULT_ON) {
    return STATE_FAULT;
  }

  if (state == STATE_IDLE) {
    if (mv >= RUN_ON) return STATE_RUNNING;
  }
  else if (state == STATE_RUNNING) {
    if (mv >= WARN_ON) return STATE_WARNING;
    if (mv <= RUN_OFF) return STATE_IDLE;
  }
  else if (state == STATE_WARNING) {
    if (mv <= WARN_OFF) return STATE_RUNNING;
  }

  return state;   // no change
}


// only change state if the new one lasts for HOLD_MS
void updateState(unsigned long now) {
  State wanted = wantedState();

  if (wanted == state) {
    pendingState = state;
    return;
  }

  if (wanted != pendingState) {
    pendingState = wanted;
    pendingSince = now;
  }
  else if (now - pendingSince >= HOLD_MS) {
    state = wanted;
  }
}


void setLeds() {
  if (state == STATE_IDLE) {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
  }
  else if (state == STATE_RUNNING) {
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);
  }
  else {
    // warning and fault
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
  }
}


void showScreen() {
  screen.clearBuffer();
  screen.setFont(u8g2_font_ncenB14_tr);

  screen.setCursor(0, 25);
  screen.print(mv, 0);
  screen.print(" mV");

  screen.setCursor(0, 55);
  if (state == STATE_IDLE)    screen.print("IDLE");
  if (state == STATE_RUNNING) screen.print("RUNNING");
  if (state == STATE_WARNING) screen.print("WARNING");
  if (state == STATE_FAULT)   screen.print("FAULT");

  screen.sendBuffer();
}