#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Define OLED display parameters
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define RE and DE pins for RS485 communication
#define RE 8
#define DE 7

// Define the Modbus request packets for NPK readings
const byte nitro[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[] = {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[] = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};

// Array to store Modbus response values
byte values[11];
SoftwareSerial mod(2, 3);

// Define Wi-Fi and MQTT parameters
const char* ssid = "your_SSID";              // your network SSID (name)
const char* password = "your_PASSWORD";      // your network password
const char* mqtt_server = "your_MQTT_BROKER_IP";
const int mqtt_port = 1883;
const char* mqtt_user = "your_MQTT_USERNAME";
const char* mqtt_password = "your_MQTT_PASSWORD";
const char* mqtt_topic = "your_MQTT_TOPIC";

// Threshold value for triggering IFTTT
const int thresholdValue = 100;

// Variables to store NPK values
byte val1, val2, val3;

// Initialize Wi-Fi and MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming MQTT messages here
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Received MQTT message on topic: ");
  Serial.print(topic);
  Serial.print(", Message: ");
  Serial.println(message);

  // Parse the message and trigger IFTTT if needed
  if (atoi(message.c_str()) > thresholdValue) {
    sendIFTTTRequest(val1, val2, val3);
  }
}

void sendIFTTTRequest(int val1, int val2, int val3) {
  // Send an HTTP request to IFTTT with trigger parameters
  String url = "https://maker.ifttt.com/trigger/your_event_name/with/key/your_IFTTT_key";
  url += "?value1=" + String(val1);
  url += "&value2=" + String(val2);
  url += "&value3=" + String(val3);

  HTTPClient http;
  http.begin(url);
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    Serial.print("IFTTT request sent. Response code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void setup() {
  Serial.begin(9600);
  mod.begin(9600);
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);

  // Initialize OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(500);
  display.clearDisplay();
  display.setCursor(25, 15);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(" NPK Sensor");
  display.setCursor(25, 35);
  display.setTextSize(1);
  display.print("Initializing");
  display.display();
  delay(3000);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT broker
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  if (client.connect("NPKClient", mqtt_user, mqtt_password)) {
    Serial.println("Connected to MQTT broker");
    client.subscribe(mqtt_topic);
  } else {
    Serial.println("MQTT connection failed");
  }
}

void loop() {
  // Read NPK values from the sensor
  val1 = nitrogen();
  delay(250);
  val2 = phosphorous();
  delay(250);
  val3 = potassium();
  delay(250);

  // Display NPK values on the OLED
  Serial.print("Nitrogen: ");
  Serial.print(val1);
  Serial.println(" mg/kg");
  Serial.print("Phosphorous: ");
  Serial.print(val2);
  Serial.println(" mg/kg");
  Serial.print("Potassium: ");
  Serial.print(val3);
  Serial.println(" mg/kg");
  delay(2000);

  display.clearDisplay();

  display.setTextSize(2);
  display.setCursor(0, 5);
  display.print("N: ");
  display.print(val1);
  display.setTextSize(1);
  display.print(" mg/kg");

  display.setTextSize(2);
  display.setCursor(0, 25);
  display.print("P: ");
  display.print(val2);
  display.setTextSize(1);
  display.print(" mg/kg");

  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print("K: ");
  display.print(val3);
  display.setTextSize(1);
  display.print(" mg/kg");

  display.display();

  // Check if any of the NPK values exceed the threshold
  if (val1 > thresholdValue || val2 > thresholdValue || val3 > thresholdValue) {
    // Trigger IFTTT request
    sendIFTTTRequest(val1, val2, val3);
  }

  // Publish NPK values to the MQTT topic
  if (client.connected()) {
    String payload = String(val1) + "," + String(val2) + "," + String(val3);
    client.publish(mqtt_topic, payload.c_str());
  }

  // Handle MQTT messages and maintain the connection
  client.loop();
  delay(1000);
}

byte nitrogen() {
  // Send Modbus request for nitrogen
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  if (mod.write(nitro, sizeof(nitro)) == 8) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    for (byte i = 0; i < 7; i++) {
      values[i] = mod.read();
      Serial.print(values[i], HEX);
    }
    Serial.println();
  }
  return values[4];
}

byte phosphorous() {
  // Send Modbus request for phosphorous
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  if (mod.write(phos, sizeof(phos)) == 8) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    for (byte i = 0; i < 7; i++) {
      values[i] = mod.read();
      Serial.print(values[i], HEX);
    }
    Serial.println();
  }
  return values[4];
}

byte potassium() {
  // Send Modbus request for potassium
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  if (mod.write(pota, sizeof(pota)) == 8) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    for (byte i = 0; i < 7; i++) {
      values[i] = mod.read();
      Serial.print(values[i], HEX);
    }
    Serial.println();
  }
  return values[4];
}
