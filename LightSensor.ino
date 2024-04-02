#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <BH1750.h>

// WiFi credentials
const char* ssid     = "iiNetA59E43";
const char* password = "2hAkfqsQkN";

// IFTTT credentials
const char* host = "maker.ifttt.com";
const int   httpPort = 80;
const String iftttEventName1 = "LightHits";
const String iftttEventName2 = "LightStops";
const String iftttKey = "bfekfVOjnING7Q4hSeTus2";

WiFiClient client;
HttpClient httpClient = HttpClient(client, host, httpPort);
BH1750 lightMeter;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  connectToWiFi();

  lightMeter.begin();
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
}


void loop() {
  static bool lightThresholdCrossed = false; // Track if we've already crossed the threshold
  float lux = lightMeter.readLightLevel();
  Serial.print("Lux: ");
  Serial.println(lux);

  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting reconnection.");
    connectToWiFi();
  }

  if (lux > 50 && !lightThresholdCrossed) {
    // If light is above threshold and we haven't already crossed the threshold
    sendNotificationHits(lux);
    Serial.println("Sunlight hits terrarium");
    lightThresholdCrossed = true; // Set the flag to true
  } else if (lux <= 50 && lightThresholdCrossed) {
    // If light is below threshold and we had previously crossed the threshold
    sendNotificationStops(lux);
    Serial.println("Sunlight stop hits terrarium");
    lightThresholdCrossed = false; // Reset the flag
  }

  delay(5000); // Check every 5 seconds
}

void sendNotificationHits(float lux) {
  Serial.println("Sending 'LightHits' notification...");
  String iftttUrl = "/trigger/" + iftttEventName1 + "/with/key/" + iftttKey;
  httpClient.post(iftttUrl, "application/json", "{\"value1\":\"" + String(lux) + "\"}");
  checkHttpResponse();
}

void sendNotificationStops(float lux) {
  Serial.println("Sending 'LightStops' notification...");
  String iftttUrl = "/trigger/" + iftttEventName2 + "/with/key/" + iftttKey;
  httpClient.post(iftttUrl, "application/json", "{\"value1\":\"" + String(lux) + "\"}");
  checkHttpResponse();
}

void checkHttpResponse() {
  int statusCode = httpClient.responseStatusCode();
  String response = httpClient.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

  if (statusCode != 200) {
    Serial.println("HTTP Request failed, check your IFTTT configuration.");
  }
}