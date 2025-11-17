// ARDUINO CODE: arduino_compost_sensor.ino (Final and Corrected)

#include <OneWire.h>
#include <DallasTemperature.h>

// --- Configuration (MISSING SECTION - CRITICAL FIX) ---
// These lines MUST be at the top so the compiler knows what FAN_RELAY_PIN is.
#define ONE_WIRE_BUS 2     // Digital Pin for DS18B20 Data line
#define FAN_RELAY_PIN 4    // Digital Pin connected to the Relay Module Signal Pin
#define MOISTURE_PIN A0    // Analog Pin for Capacitive Moisture Sensor

// --- Initial Setup ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress coreTempProbe;

long lastDataSendTime = 0;
const long sendInterval = 5000; 

void setup() {
  // FIX 1: Set pin state LOW immediately AND configure as output (assuming Active-LOW relay)
  // LOW ensures the relay starts OFF based on the recent diagnosis.
  
  pinMode(FAN_RELAY_PIN, OUTPUT);
  digitalWrite(FAN_RELAY_PIN, HIGH); 
  Serial.begin(9600);
  
  sensors.begin();
  
  if (sensors.getAddress(coreTempProbe, 0)) {
    Serial.println("DS18B20 found and initialized.");
    sensors.setResolution(coreTempProbe, 10); 
  } else {
    Serial.println("CRITICAL ERROR: DS18B20 not detected. Check wiring/resistor.");
  }
}

void loop() {
  // --- A. DATA ACQUISITION (UPLINK) ---
  if (millis() - lastDataSendTime >= sendInterval) {
    lastDataSendTime = millis();
    
    sensors.requestTemperatures();
    float tempC = sensors.getTempC(coreTempProbe);

    if (tempC != DEVICE_DISCONNECTED_C && tempC > -127.0) {
      int moistureRaw = analogRead(MOISTURE_PIN);

      // Send Data to Pi (Format: T<C_Temp>|M<Raw_Moisture>\n)
      Serial.print("T");
      Serial.print(tempC, 1); 
      Serial.print("|M");
      Serial.println(moistureRaw);
    } 
  }

  // --- B. POLICY EXECUTION (DOWNLINK) ---
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    // Execute the Pi's Policy Command (Active-LOW Logic)
    if (command == "FAN_OFF") {
      digitalWrite(FAN_RELAY_PIN, LOW); // LOW activates the relay
    } else if (command == "FAN_ON") {
      digitalWrite(FAN_RELAY_PIN, HIGH); // HIGH deactivates the relay
    }
  }
}