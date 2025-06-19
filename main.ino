// ✅ Blynk Credentials
#define BLYNK_TEMPLATE_ID "TMPL3n61Gre25"
#define BLYNK_TEMPLATE_NAME "Irrigation"
#define BLYNK_AUTH_TOKEN "kbgj5uh6n0tk0dD1uHv0wG3WIS19KIRW"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include "ThingSpeak.h"
#include <ESP32Servo.h>

// ✅ WiFi Credentials
const char* ssid = "Nishant";
const char* password = "12121212";

// ✅ ThingSpeak Credentials
const char* tsServer = "api.thingspeak.com";
unsigned long myChannelNumber = 2888465;
const char* myWriteAPIKey = "VIGJTXWHDW6717M9";

// 🌱 Sensor Pins
#define SOIL_MOISTURE_PIN 34
#define AIR_QUALITY_PIN 35
#define DHT_PIN 32
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

// ✅ Virtual Pins
#define V_SOIL_MOISTURE V3
#define V_AIR_QUALITY V4
#define V_TEMP V5
#define V_HUMIDITY V6
#define V_VALVE_CONTROL V0 // Servo control

// ✅ Servo Motor Setup
#define SERVO_PIN 18  // Use a PWM-capable pin
Servo valveServo;

// ✅ ThingSpeak Setup
WiFiClient client;

// ✅ Timer for Non-blocking Delay
unsigned long lastUpdateTime = 0; // Stores last update timestamp
const long updateInterval = 15000; // 15 seconds

void setup() {
    Serial.begin(115200);
    
    // ✅ Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");

    // ✅ Start Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

    // ✅ Start ThingSpeak
    ThingSpeak.begin(client);

    // ✅ Initialize Sensors
    dht.begin();

    // ✅ Initialize Servo
    valveServo.attach(SERVO_PIN);
    valveServo.write(0);  // Start with valve closed
    Serial.println("Servo initialized.");
}

void loop() {
    Blynk.run(); // Keep Blynk running for real-time control

    // ✅ Non-blocking timer for ThingSpeak updates
    if (millis() - lastUpdateTime >= updateInterval) {
        lastUpdateTime = millis();
        readSensors();
    }
}

// ✅ Read Sensors & Send Data
void readSensors() {
    int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
    int airQuality = analogRead(AIR_QUALITY_PIN);
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    Serial.print("🌱 Soil Moisture: "); Serial.println(soilMoisture);
    Serial.print("🛑 Air Quality: "); Serial.println(airQuality);
    Serial.print("🌡 Temperature: "); Serial.println(temperature);
    Serial.print("💧 Humidity: "); Serial.println(humidity);

    // ✅ Send Data to Blynk
    Blynk.virtualWrite(V_SOIL_MOISTURE, soilMoisture);
    Blynk.virtualWrite(V_AIR_QUALITY, airQuality);
    Blynk.virtualWrite(V_TEMP, temperature);
    Blynk.virtualWrite(V_HUMIDITY, humidity);

    // ✅ Send Data to ThingSpeak
    ThingSpeak.setField(1, soilMoisture);
    ThingSpeak.setField(2, airQuality);
    ThingSpeak.setField(3, temperature);
    ThingSpeak.setField(4, humidity);
    
    int tsResponse = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (tsResponse == 200) {
        Serial.println("✅ ThingSpeak update successful!");
    } else {
        Serial.print("❌ ThingSpeak update failed. Error code: ");
        Serial.println(tsResponse);
    }
}

// ✅ Blynk Valve Control Function (Works Instantly)
BLYNK_WRITE(V_VALVE_CONTROL) {
    int valveState = param.asInt(); // Read Button State from Blynk App
    if (valveState == 1) {
        Serial.println("🚰 Opening valve...");
        valveServo.write(90); // Open valve
    } else {
        Serial.println("❌ Closing valve...");
        valveServo.write(0); // Close valve
    }
}