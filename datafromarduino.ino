#include <ArduinoMqttClient.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
#include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
#include <WiFi101.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_GIGA)
#include <WiFi.h>
#endif
#include <DHT.h>
#include <DHT_U.h>
#include <ArduinoHttpClient.h>
#include <Adafruit_Sensor.h>
//#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "@prabh";  // your network SSID (name)
char pass[] = "0987654321";  // your network password (use for WPA, or use as key for WEP)
lIgWNjFVMYsfl7hyyEZptkeyHhbhKZllfTEbWWBclsZ
char HOST_NAME[]="maker.ifttt.com";
String PATH_NAME="/trigger/sensor_data/with/key/lIgWNjFVMYsfl7hyyEZptkeyHhbhKZllfTEbWWBclsZ
char firebaseHost[] = "finalproject-8c54c-default-rtdb.firebaseio.com";
; // Replace with your Firebase project's database URL (without "https://")
String apiKey = "AIzaSyD1DZito8fL7Kpkrg27rY1TSOtasFm6weg"; // Replace with your Firebase project's API key
const int sensorPin = A0;  // Soil moisture sensor pin
const int DHTPin = 2;     // Digital pin connected to the DHT22 sensor
const int DHTType = DHT22; // Change to DHT11 if you're using that sensor
WiFiSSLClient wifiClient;
HttpClient client = HttpClient(wifiClient, firebaseHost,443);
DHT_Unified dht(DHTPin, DHTType);

MqttClient mqttClient(wifiClient);

const char broker[] = "broker.hivemq.com";
int port = 1883;
const char topic[] = "Data";

const long interval = 1000;
unsigned long previousMillis = 0;

const int turbidityPin = A0;  // Change this to the appropriate pin
float pHValue, sensorValue = 0;
void sendDataToFirebase(float temperature, float humidity, float moisture_percentage) 
{
  if (client.connect(firebaseHost,443)) { // Use client.connect() with host and port
    // Construct the Firebase URL with the path where you want to store data
    String firebasePath = "/sensorData.json"; // Replace with your desired path
    String url = "/" + firebasePath + "?auth=" + apiKey;

    // Send the data to Firebase using HTTP PUT or POST
    String dataToSend = "{\"temperature\": " + String(temperature, 2) + ", \"humidity\": " + String(humidity, 2) +", \"Moisture\": " + String(moisture_percentage, 2)+"}";
    client.put(url, "application/json", dataToSend);

    // Check for the server's response
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    Serial.print("HTTP Status Code: ");
    Serial.println(statusCode);
    Serial.print("Server Response: ");
    Serial.println(response);

    client.stop();
  } else {
    Serial.println("DATA NOT SENT");
  }
}
void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to WiFi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  // mqttClient.setId("clientId");

  // You can provide a username and password for authentication
  // mqttClient.setUsernamePassword("username", "password");

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1)
      ;
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
   dht.begin();   
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  // to avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
 float moisturePercentage;
  int sensorAnalog;

  // Read soil moisture sensor
  sensorAnalog = analogRead(sensorPin);
  moisturePercentage = 100 - (sensorAnalog / 10.23); // Adjust the scale if needed

  // Read DHT22 sensor data
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(event.temperature);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(event.relative_humidity);
    Serial.println("%");
  }
 sendDataToFirebase(event.temperature, event.relative_humidity,moisturePercentage);
  // Print soil moisture data
  Serial.print("Moisture Percentage: ");
  Serial.print(moisturePercentage);
  Serial.println("%");

    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print(event.temperature);
    mqttClient.endMessage();

    Serial.println();

  }

  delay(1000);
}