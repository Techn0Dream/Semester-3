#include <WiFiNINA.h>
#include <PubSubClient.h>

// ---------- WiFi Credentials ----------
const char* WIFI_SSID = "RedmiNote13Pro5G";
const char* WIFI_PASSWORD = "Yogya@214";

// ---------- MQTT Broker ----------
const char* MQTT_BROKER = "broker.emqx.io";
const int   MQTT_PORT   = 1883;
const char* MQTT_TOPIC  = "SIT210/wave";

// ---------- Hardware Pins ----------
const int TRIG_PIN = 2;
const int ECHO_PIN = 3;
const int LED_PIN  = 5;

// ---------- Globals ----------
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// ---------- Distance Function ----------
long getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.034 / 2; // speed of sound (cm/us)
  return distance;
}

// ---------- MQTT Callback ----------
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // LED Pattern
  if (message.indexOf("wave") != -1) {
    // Wave: blink 3 times
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(300);
      digitalWrite(LED_PIN, LOW);
      delay(300);
    }
  } else if (message.indexOf("pat") != -1) {
    // Pat: different blink (2 long blinks)
    for (int i = 0; i < 2; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(800);
      digitalWrite(LED_PIN, LOW);
      delay(400);
    }
  }
}

// ---------- Reconnect to MQTT ----------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "Nano33IoT-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(MQTT_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// ---------- Setup ----------
void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Connect to WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected!");

  // Setup MQTT
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(mqttCallback);
}

// ---------- Loop ----------
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Wave detection
  long distance = getDistanceCM();
  if (distance > 0 && distance < 20) { // hand within 20cm
    Serial.println("Wave detected!");

    // Publish with name included
    String payload = "{\"sender\":\"Yogya Arora\",\"type\":\"wave\"}";
    client.publish(MQTT_TOPIC, payload.c_str());
    delay(2000); // avoid spamming
  }
}
