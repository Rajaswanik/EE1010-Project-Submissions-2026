#include <LiquidCrystal.h>

// LCD Pin connections matching hardware table
// RS -> D7 (Pin 22), E -> D6 (Pin 21), DB4 -> D5 (Pin 20), DB5 -> D4 (Pin 19), DB6 -> D3 (Pin 18), DB7 -> D2 (Pin 17)
const int RS_PIN = 7;
const int EN_PIN = 6;
const int DB4_PIN = 5;
const int DB5_PIN = 4;
const int DB6_PIN = 3;
const int DB7_PIN = 2;

LiquidCrystal lcd(RS_PIN, EN_PIN, DB4_PIN, DB5_PIN, DB6_PIN, DB7_PIN);

// Hardware Pins & Circuit Parameters
const int SENSOR_PIN = A0;   // Pin 9 -> Voltage Divider Output Node
const float R1 = 100.0;      // R1 bottom resistor = 100 ohms

// PT100 Reference Calibration Dataset (IEC 60751 Standard)
const int N = 6;
const float R_data[N] = {100.00, 107.79, 115.54, 123.24, 130.89, 138.51}; // Resistance in Ohms
const float T_data[N] = {0.0,    20.0,   40.0,   60.0,   80.0,   100.0};  // Temperature in °C

float slope_m = 0.0;
float intercept_c = 0.0;

// Function to calculate slope (m) and intercept (c) using Ordinary Least Squares
void calculateLeastSquares(const float x[], const float y[], int n, float &m, float &c) {
  float sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;

  for (int i = 0; i < n; i++) {
    sum_x  += x[i];
    sum_y  += y[i];
    sum_xy += x[i] * y[i];
    sum_x2 += x[i] * x[i];
  }

  m = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
  c = (sum_y - m * sum_x) / n;
}

void setup() {
  lcd.begin(16, 2);

  // Perform Least Squares Regression fit during setup
  calculateLeastSquares(R_data, T_data, N, slope_m, intercept_c);

  lcd.print("PT100 Calibrated");
  lcd.setCursor(0, 1);
  lcd.print("Least Squares OK");
  delay(1500);
  lcd.clear();
}

void loop() {
  int adcValue = analogRead(SENSOR_PIN);

  if (adcValue > 0 && adcValue < 1023) {
    // Voltage divider formula: ADC = 1023 * (R1 / (R_pt100 + R1))
    // Solving for R_pt100: R_pt100 = R1 * ((1023.0 / ADC) - 1.0)
    float R_pt100 = R1 * ((1023.0 / (float)adcValue) - 1.0);

    // Calculate temperature using Least Squares linear model
    float temperature = (slope_m * R_pt100) + intercept_c;

    // Line 1: Resistance
    lcd.setCursor(0, 0);
    lcd.print("R: ");
    lcd.print(R_pt100, 2);
    lcd.print(" Ohm   ");

    // Line 2: Calculated Temperature
    lcd.setCursor(0, 1);
    lcd.print("Temp: ");
    lcd.print(temperature, 2);
    lcd.print(" ");
    lcd.write(223); // Degree symbol
    lcd.print("C  ");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error   ");
  }

  delay(500);
}
