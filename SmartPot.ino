#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Pin definitions
#define SOIL_SENSOR_PIN 32    // Analog soil sensor
#define WATER_LEVEL_PIN 33    // Digital water level sensor
#define PUMP_RELAY_PIN 4      // Pump control
#define BUZZER_PIN 16         // Buzzer
#define RED_LED_PIN 15        // Status LED
#define GREEN_LED_PIN 2       // Status LED
#define LDR_LEFT_PIN 12       // Left light sensor
#define LDR_RIGHT_PIN 13      // Right light sensor

// Stepper motor setup
const int STEPS_PER_REV = 2048;
Stepper stepper(STEPS_PER_REV, 14, 27, 26, 25);

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Web server on port 80
WebServer server(80);

// System state variables
bool manualMode = false;
bool manualWatering = false;
int manualRotation = 0;
unsigned long pumpStartTime = 0;
const long PUMP_TIMEOUT = 10000; // 10 seconds

void setup() {
    Serial.begin(115200);

    // Initialize I/O
    pinMode(PUMP_RELAY_PIN, OUTPUT);
    pinMode(WATER_LEVEL_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);

    // Motor and display setup
    stepper.setSpeed(10);
    lcd.init();
    lcd.backlight();

    // Connect to WiFi
    WiFi.begin(ssid, password);
    lcd.print("Connecting WiFi...");
    while (WiFi.status() != WL_CONNECTED) delay(500);

    lcd.clear();
    lcd.print("IP: ");
    lcd.print(WiFi.localIP());

    // Configure web routes
    server.on("/", handleRoot);
    server.on("/water", handleWater);
    server.on("/rotate", handleRotate);
    server.begin();
}

void loop() {
    server.handleClient();
    handleWaterLevel();

    if (!manualMode) {
        // Automatic mode operations
        int moisture = analogRead(SOIL_SENSOR_PIN);
        bool waterLow = !digitalRead(WATER_LEVEL_PIN);

        digitalWrite(PUMP_RELAY_PIN, (moisture > 2500 && !waterLow) ? HIGH : LOW);

        int lightDiff = analogRead(LDR_RIGHT_PIN) - analogRead(LDR_LEFT_PIN);
        if (abs(lightDiff) > 100) {
            stepper.step(lightDiff > 0 ? 50 : -50);
        }
    }
    else {
        // Manual control
        digitalWrite(PUMP_RELAY_PIN, manualWatering ? HIGH : LOW);

        if (manualRotation != 0) {
            stepper.step(manualRotation * 50);
        }

        if (manualWatering && millis() - pumpStartTime > PUMP_TIMEOUT) {
            manualWatering = false;
            digitalWrite(PUMP_RELAY_PIN, LOW);
        }
    }

    updateDisplay();
    delay(100);
}

// Handle water level alerts
void handleWaterLevel() {
    bool waterLow = !digitalRead(WATER_LEVEL_PIN);

    if (waterLow) {
        digitalWrite(BUZZER_PIN, HIGH);
        digitalWrite(RED_LED_PIN, (millis() % 500 < 250));
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(PUMP_RELAY_PIN, LOW);
        manualWatering = false;
    }
    else {
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(GREEN_LED_PIN, !manualMode);
    }
}

// Update LCD display
void updateDisplay() {
    lcd.setCursor(0, 0);
    lcd.print("Moist: ");
    lcd.print(map(analogRead(SOIL_SENSOR_PIN), 0, 4095, 0, 100));
    lcd.print("%  ");

    lcd.setCursor(0, 1);
    lcd.print(manualMode ? "MANUAL " : "AUTO    ");
    lcd.print(digitalRead(WATER_LEVEL_PIN) ? "OK" : "LOW");
}

// Web handlers
void handleRoot() {
    String page = "<html><body><h1>Plant Control</h1>";
    page += "<p>Mode: " + String(manualMode ? "Manual" : "Auto") + "</p>";
    page += "<button onclick=\"fetch('/water?enable=1')\">Start Water</button>";
    page += "<button onclick=\"fetch('/water?enable=0')\">Stop Water</button>";
    server.send(200, "text/html", page);
}

void handleWater() {
    manualMode = true;
    manualWatering = server.arg("enable") == "1";
    pumpStartTime = millis();
    server.send(200, "text/plain", "OK");
}

void handleRotate() {
    manualMode = true;
    manualRotation = server.arg("dir").toInt();
    server.send(200, "text/plain", "OK");
}
