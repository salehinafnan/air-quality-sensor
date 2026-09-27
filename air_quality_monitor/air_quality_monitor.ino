/*
 * Air Quality Monitor
 * Arduino Uno + MQ-135 gas sensor + 16x2 character LCD
 *
 * Reads the MQ-135 analog output on A0, averages several samples to reduce
 * noise, shows the level on the LCD and classifies the air as good or bad
 * against a threshold. Every reading is also sent to the Serial Monitor /
 * Serial Plotter at 9600 baud.
 *
 * The value shown is the raw 10-bit ADC reading (0-1023). It rises with the
 * concentration of gases the MQ-135 reacts to (CO2, NH3, NOx, alcohol, smoke,
 * benzene), but it is not a calibrated ppm figure. See the README for details.
 */

#include <LiquidCrystal.h>

// LCD wiring: RS, E, D4, D5, D6, D7 (RW is tied to GND)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const uint8_t GAS_PIN = A0;
const int THRESHOLD = 250;           // readings above this are flagged as bad air
const int HYSTERESIS = 10;           // stops the status flickering around the threshold
const uint8_t SAMPLES = 10;          // analog reads averaged per update
const unsigned long UPDATE_MS = 400; // display refresh interval
const unsigned long WARMUP_MS = 0;   // MQ-135 heater warm-up; use ~60000 on real hardware

bool badAir = false;

int readGasLevel() {
  long sum = 0;
  for (uint8_t i = 0; i < SAMPLES; i++) {
    sum += analogRead(GAS_PIN);
    delay(2);
  }
  return sum / SAMPLES;
}

void setup() {
  pinMode(GAS_PIN, INPUT);
  lcd.begin(16, 2);
  Serial.begin(9600);

  if (WARMUP_MS > 0) {
    lcd.print("Air Quality Mon.");
    lcd.setCursor(0, 1);
    lcd.print("Warming up...");
    delay(WARMUP_MS);
    lcd.clear();
  }
}

void loop() {
  int level = readGasLevel();

  if (level > THRESHOLD) {
    badAir = true;
  } else if (level < THRESHOLD - HYSTERESIS) {
    badAir = false;
  }

  // Overwrite both lines in place instead of calling lcd.clear(), which flickers
  char line[17];
  snprintf(line, sizeof(line), "Gas level: %4d ", level);
  lcd.setCursor(0, 0);
  lcd.print(line);
  lcd.setCursor(0, 1);
  lcd.print(badAir ? "Bad Air Quality " : "Good Air Quality");

  // "label:value" pairs render as separate traces in the Serial Plotter
  Serial.print("Gas:");
  Serial.print(level);
  Serial.print("\tThreshold:");
  Serial.println(THRESHOLD);

  delay(UPDATE_MS);
}
