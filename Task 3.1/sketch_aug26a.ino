#include <Wire.h>
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include <BH1750.h>

const char* WIFI_SSID  = "RedmiNote13Pro5G";
const char* WIFI_PASSWORD = "Yogya@214";

const char* EVENT_START = "Sunlight_start";
const char* EVENT_STOP  = "Sunlight_stop";
const char* IFTTT_KEY   = "eJZPPlW1KGK6nE-OI6nWehjGL7yRmvaFof0K5EJlnLw";

const float LIGHT_THRESHOLD = 500.0;

BH1750 lightMeter;
float luxLevel = 0;
float sunlightMinutes = 0;
bool light = false;
bool previousLight = false;
unsigned long lastMillis = 0;

WiFiClient wifiClient;

void setup() {
  Serial.begin(9600);
  delay(1500);

  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Wire.begin();
  if (lightMeter.begin()) {
    Serial.println("BH1750 initialized ");
  } else {
    Serial.println(" Error initializing BH1750");
    while (1);
  }

  lastMillis = millis();
}

void loop() {
  luxLevel = lightMeter.readLightLevel();
  Serial.print("Lux: "); Serial.println(luxLevel);

  light = luxLevel >= LIGHT_THRESHOLD;

  if (light != previousLight) {
    if (light) {
      triggerIFTTT(EVENT_START, luxLevel);
      Serial.println("☀ Sunlight started - IFTTT triggered");
    } else {
      triggerIFTTT(EVENT_STOP, sunlightMinutes);
      Serial.println(" Sunlight stopped - IFTTT triggered");
    }
    previousLight = light;
  }

  if (light) {
    unsigned long now = millis();
    float deltaMinutes = (now - lastMillis) / 60000.0; 
    sunlightMinutes += deltaMinutes;
    lastMillis = now;
    Serial.print("Sunlight minutes: "); Serial.println(sunlightMinutes);
  } else {
    lastMillis = millis();
  }

  delay(1000);
}

void triggerIFTTT(const char* eventName, float value) {
  HttpClient http(wifiClient, "maker.ifttt.com", 80);

  String url = "/trigger/";
  url += eventName;
  url += "/with/key/";
  url += IFTTT_KEY;
  url += "?value1=";
  url += String(value);

  http.get(url);
  int status = http.responseStatusCode();
  String response = http.responseBody();

  Serial.print("IFTTT Status: "); Serial.println(status);
  Serial.print("IFTTT Response: "); Serial.println(response);

  if (status == 200) {
    Serial.println(" IFTTT Trigger Sent!");
  } else {
    Serial.println(" Error Sending to IFTTT");
  }
}