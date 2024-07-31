#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <GxEPD2_BW.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include "config.h"

// NTP-Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000); // 3600 Sekunden = 1 Stunde

// Globale Variablen für MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Define the display
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(/*CS=*/ 10, /*DC=*/ 9, /*RST=*/ 8, /*BUSY=*/ 7));

String temperature = "N/A";

void connectToWiFi() {
  display.clearScreen();
  display.print("Connecting to WiFi...");
  display.display();
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    display.println("\nConnected to WiFi");
  } else {
    Serial.println("\nFailed to connect to WiFi");
    display.println("\nFailed to connect to WiFi");
  }
  display.display();
}

void mqtt_reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(MQTT_ID)) {
      Serial.println("connected");
      client.subscribe(MQTT_TOPIC);
      Serial.print("Subscribed to topic: ");
      Serial.println(MQTT_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
void displayContent() {
  // Fetch the time from the NTP-Server
  timeClient.update();

  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);

  ptm->tm_hour = (ptm->tm_hour + 1) % 24; // Adjust timezone offset if needed

  int day = ptm->tm_mday;
  int month = ptm->tm_mon + 1;
  int year = ptm->tm_year + 1900;
  int hour = ptm->tm_hour;
  int minute = ptm->tm_min;

  // Format the date and time
  char dateBuffer[16];
  snprintf(dateBuffer, sizeof(dateBuffer), "%02d.%02d.%04d", day, month, year);

  char timeBuffer[16];
  snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d", hour, minute);

  // Update the display
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.setTextColor(GxEPD_BLACK);
    display.println(String(dateBuffer) + "              " + String(timeBuffer) + " Uhr");
    display.println("Außentemperatur: " + temperature + "°C");
  } while (display.nextPage());
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");
  // for (unsigned int i = 0; i < length; i++) {
  //   Serial.print((char)payload[i]);
  // }
  // Serial.println();
  // Hier können Sie die Nachrichtenverarbeitung hinzufügen
  if (strcmp(topic, MQTT_TOPIC) == 0) {
    // tempertur = "Außentemperatur: " + (char *)payload;
    float temp = atof((char *)payload);
    temperature = String(temp, 1);
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Set up the SPI bus
  SPI.begin(/*SCK=*/ 13, /*MISO=*/ -1, /*MOSI=*/ 11, /*SS=*/ 10);

  // Initialize the display
  display.init(115200);

  // Setup WiFi
  display.nextPage();
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(GxEPD_BLACK);
  display.display();

  connectToWiFi();

  // Initialize NTP-Client
  timeClient.begin();

  // MQTT-Server setzen
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(mqtt_callback);
}

void loop() {
  // Ensure WiFi connection is active
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  } else {
    displayContent();
  }

  // Ensure MQTT client loop is called
  if (!client.connected()) {
    mqtt_reconnect();
  }
  client.loop();

  delay(1000);
}
