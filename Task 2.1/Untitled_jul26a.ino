#include "arduino_secrets.h"
#include <SPI.h>
#include <WiFiNINA.h>
#include "DHT.h"
#include <ArduinoHttpClient.h>

char ssid[] = "RedmiNote13Pro5G";       
char pass[] = "Yogya@214";       
int status = WL_IDLE_STATUS;

char server[] = "https://api.thingspeak.com/";
String apiKey = "B2RVGBDD5KKUJUU4"; 

WiFiClient client;
HttpClient httpClient = HttpClient(client, server, 80);

#define DHTPIN 2      
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  dht.begin();

  while (status != WL_CONNECTED) {
    Serial.print("Connecting to WiFi...");
    status = WiFi.begin(ssid, pass);
    delay(5000);
  }
  Serial.println("WiFi connected");
}

void loop() {
  float temp = dht.readTemperature();  // Celsius

  if (isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.println(temp);

  String postData = "api_key=" + apiKey + "&field1=" + String(temp);

  httpClient.post("/update", "application/x-www-form-urlencoded", postData);
  int statusCode = httpClient.responseStatusCode();
  String response = httpClient.responseBody();

  Serial.print("Status Code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

  delay(30000);  
}