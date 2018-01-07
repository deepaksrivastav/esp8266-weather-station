/**
 * BME280-SSD1306 OLED-Weather Station
 *
 * By: Deepak Srivastav
 * Date: 24 Dec 2017
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SSD1306.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// For calculating altitute - Future use
// #define SEA_LEVEL_PRESSURE_HPA (1013.25)

// WLAN Details
#define WLAN_SSID "WLAN_SSID_HERE"
#define WLAN_PASS "WLAN_PASSWORD_HERE"

// MQTT Broker Details
#define MQTT_PORT 1883
#define MQTT_SERVER "mqtt.host.local"
#define MQTT_USERNAME "mqtt_username"
#define MQTT_PASSWORD "mqtt_password"
#define TEMPERATURE_FEED "sensor/esp8266/temperature"
#define HUMIDITY_FEED "sensor/esp8266/humidity"

// read data every minute
const int DELAY = 60000;
const int STARTUP_DELAY = 500;

String temp_str;
String hum_str;
char temp[50];
char hum[50];

// BME280 sensor
Adafruit_BME280 bme;

// SSD1306 display
SSD1306 display(0x3c, D2, D1);

// WiFi Client
WiFiClient espClient;

// MQTT Client
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  Serial.println("Looking for BME280..");
  if(!bme.begin())
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
    {
        yield();
        delay(DELAY);
    }
  }

  // Connect to WiFi access point.
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

  }
  Serial.println();

  client.setServer(MQTT_SERVER, MQTT_PORT);

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  delay(STARTUP_DELAY);

  display.init();
  display.flipScreenVertically();

}

void mqttConnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266 Client", MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    mqttConnect();
  }
  client.loop();

  float tempC = bme.readTemperature();
  float humidity = bme.readHumidity();

  // Print to serial monitor
  printToSerial(tempC, humidity);

  // display on the OLED Screen
  displayData(tempC, humidity);

  // publish to mqttConnect
  publishToMqtt(tempC, humidity);

  yield();
  delay(DELAY);
}

void publishToMqtt(float tempC, float humidity) {
  // convert temperature and humidity to strings
  temp_str = String(tempC);
  temp_str.toCharArray(temp, temp_str.length() + 1);

  hum_str = String(humidity);
  hum_str.toCharArray(hum, hum_str.length() + 1);

  client.publish(TEMPERATURE_FEED, temp);
  client.publish(HUMIDITY_FEED, hum);
}

void displayData(float tempC, float humidity) {
  display.clear();
  // draw a rectangle border to the display
  display.drawRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  display.setFont(ArialMT_Plain_24);
  display.drawString(20, 3, String(tempC) + " *C");
  display.drawString(20, 30, String(humidity) + " %");
  display.display();
}

void printToSerial(float tempC, float humidity) {
  // Temperature
  Serial.println("Temperature:");
  printValueAndUnits(tempC, "*C");
  Serial.println("");

  // Humidity
  Serial.println("Humidity:");
  printValueAndUnits(humidity, "%");
  Serial.println("");
}

void printValueAndUnits(float value, String units) {
  Serial.print("     ");
  Serial.print(value);
  Serial.print(" ");
  Serial.println(units);
}
