#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>

// Pin definitions
#define SOIL_SENSOR_PIN A0
#define WATER_LEVEL_PIN 2
#define PUMP_RELAY_PIN 3
#define BUZZER_PIN 4
#define RED_LED_PIN 5
#define GREEN_LED_PIN 6
#define LDR_LEFT_PIN A1
#define LDR_RIGHT_PIN A2
#define CONTROL_BTN 7

// Threshold values
#define MIN_MOISTURE 300
#define MAX_MOISTURE 800

// System objects
LiquidCrystal_I2C lcd(0x27, 16, 2);
Stepper motor(2048, 8, 9, 10, 11);

// System state
volatile bool manualMode = false;
int soilMoisture = 0;

// Display update function
void updateDisplay() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Moist: ");
    lcd.print(map(soilMoisture, MIN_MOISTURE, MAX_MOISTURE, 0, 100));
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print(manualMode ? "MANUAL " : "AUTO   ");
    lcd.print(digitalRead(WATER_LEVEL_PIN) ? "OK" : "LOW");
}

// ADC interrupt handler
ISR(ADC_vect) {
    soilMoisture = ADC;
}

// Mode button interrupt
ISR(INT0_vect) {
    manualMode = !manualMode;
}

// ADC initialization
void setupADC() {
    ADMUX |= (1 << REFS0); // AVcc reference
    ADMUX |= (0 << MUX0);  // ADC0 channel
    ADCSRA |= (1 << ADEN) | (1 << ADIE) | (7 << ADPS0); // Enable ADC with interrupt
    ADCSRA |= (1 << ADSC); // Start conversion
}

void setup() {
    // Pin configuration
    pinMode(WATER_LEVEL_PIN, INPUT_PULLUP);
    pinMode(PUMP_RELAY_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);

    // Interrupt setup
    EICRA |= (1 << ISC01); // Falling edge trigger
    EIMSK |= (1 << INT0);  // Enable INT0

    // System initialization
    setupADC();
    motor.setSpeed(10);
    lcd.init();
    lcd.backlight();
}

void loop() {
    static unsigned long lastUpdate = 0;

    // Update sensors every second
    if (millis() - lastUpdate > 1000) {
        ADCSRA |= (1 << ADSC);
        lastUpdate = millis();
    }

    // System operations
    if (!manualMode) {
        // Automatic control
        bool needsWater = soilMoisture < MIN_MOISTURE && digitalRead(WATER_LEVEL_PIN);
        digitalWrite(PUMP_RELAY_PIN, needsWater);

        int lightDiff = analogRead(LDR_RIGHT_PIN) - analogRead(LDR_LEFT_PIN);
        if (abs(lightDiff) > 100) {
            motor.step(lightDiff > 0 ? 50 : -50);
        }
    }

    // Update display and handle LEDs
    updateDisplay();
    digitalWrite(GREEN_LED_PIN, !manualMode);
    digitalWrite(RED_LED_PIN, digitalRead(WATER_LEVEL_PIN) ? LOW : HIGH);
    delay(100);
}
