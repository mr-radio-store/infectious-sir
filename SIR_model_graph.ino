/*
Classic Compartmental Model: SIR Model 

***
SIR Model Overview

The population is divided into three compartments at time tt:
    S(t)S(t): Number of Susceptible individuals (can catch the disease)
    I(t)I(t): Number of Infectious individuals (currently infected and can transmit)
    R(t)R(t): Number of Recovered (or Removed) individuals (immune or no longer infectious)
The total population NN is assumed constant:
N=S(t)+I(t)+R(t)
N=S(t)+I(t)+R(t)
***

OLED wire connecction
Wiring: I2C OLED
OLED Pin	Arduino Uno / Mega
VCC	5V
GND	GND
SDA	A4 (Uno) / 20 (Mega)
SCL	A5 (Uno) / 21 (Mega)
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int totalPopulation = 1000;
float S = totalPopulation - 1;
float I = 1;
float R = 0;

const float beta = 0.3;
const float gamma = 0.1;

const int maxTimeSteps = 64;

uint8_t S_history[maxTimeSteps];
uint8_t I_history[maxTimeSteps];
uint8_t R_history[maxTimeSteps];

int timeStep = 0;

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));  // Initialize random generator

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SIR Model Sim");
  display.display();
  delay(1500);

  for (int i = 0; i < maxTimeSteps; i++) {
    S_history[i] = 0;
    I_history[i] = 0;
    R_history[i] = 0;
  }
}

void loop() {
  // Base deterministic SIR update
  float newInfections = beta * S * I / totalPopulation;
  float newRecoveries = gamma * I;

  S -= newInfections;
  I += newInfections - newRecoveries;
  R += newRecoveries;

  // Add random fluctuation factor to infectious count (-5 to +5)
  float randomFactor = (random(-50, 51)) / 10.0; // -5.0 to +5.0
  I += randomFactor;

  // Clamp values
  if (S < 0) S = 0;
  if (I < 0) I = 0;
  if (R > totalPopulation) R = totalPopulation;

  if (S + I + R > totalPopulation) {
    // Normalize to totalPopulation if sum exceeds
    float total = S + I + R;
    S = S * totalPopulation / total;
    I = I * totalPopulation / total;
    R = R * totalPopulation / total;
  }

  uint8_t S_scaled = (uint8_t)((S / totalPopulation) * 255);
  uint8_t I_scaled = (uint8_t)((I / totalPopulation) * 255);
  uint8_t R_scaled = (uint8_t)((R / totalPopulation) * 255);

  if (timeStep < maxTimeSteps) {
    S_history[timeStep] = S_scaled;
    I_history[timeStep] = I_scaled;
    R_history[timeStep] = R_scaled;
    timeStep++;
  } else {
    for (int i = 1; i < maxTimeSteps; i++) {
      S_history[i - 1] = S_history[i];
      I_history[i - 1] = I_history[i];
      R_history[i - 1] = R_history[i];
    }
    S_history[maxTimeSteps - 1] = S_scaled;
    I_history[maxTimeSteps - 1] = I_scaled;
    R_history[maxTimeSteps - 1] = R_scaled;
  }

  drawGraph();
  delay(200);
}

void drawGraph() {
  display.clearDisplay();

  String trendText = "Infectious: ";
  if (timeStep < 2) {
    trendText += "N/A";
  } else {
    if (I_history[timeStep - 1] > I_history[timeStep - 2]) {
      trendText += "increasing";
    } else if (I_history[timeStep - 1] < I_history[timeStep - 2]) {
      trendText += "decreasing";
    } else {
      trendText += "stable";
    }
  }

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(trendText);

  int graphHeight = SCREEN_HEIGHT - 12;
  int baseY = SCREEN_HEIGHT - 1;

  display.drawLine(0, baseY, SCREEN_WIDTH, baseY, SSD1306_WHITE);

  for (int i = 1; i < timeStep; i++) {
    int x1 = i - 1;
    int x2 = i;

    int yS1 = baseY - ((S_history[i - 1] * graphHeight) >> 8);
    int yS2 = baseY - ((S_history[i] * graphHeight) >> 8);

    int yI1 = baseY - ((I_history[i - 1] * graphHeight) >> 8);
    int yI2 = baseY - ((I_history[i] * graphHeight) >> 8);

    int yR1 = baseY - ((R_history[i - 1] * graphHeight) >> 8);
    int yR2 = baseY - ((R_history[i] * graphHeight) >> 8);

    display.drawLine(x1, yS1, x2, yS2, SSD1306_WHITE);
    display.drawLine(x1, yI1, x2, yI2, SSD1306_WHITE);

    if (i % 2 == 0) {
      display.drawLine(x1, yR1, x2, yR2, SSD1306_WHITE);
    }
  }

  display.display();
}
