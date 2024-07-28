#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include "config.h"

// NTP-Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000); // 3600 Sekunden = 1 Stunde

// Define the display
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(/*CS=*/ 10, /*DC=*/ 9, /*RST=*/ 8, /*BUSY=*/ 7));

void connectToWiFi() {
  display.print("Connecting to WiFi...");
  display.display();
  
  WiFi.begin(ssid, password);

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

void displayTime() {
  // Fetch the time from the NTP-Server
  timeClient.update();

  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);

  ptm->tm_hour = (ptm->tm_hour + hourOffset) % 24;

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
  } while (display.nextPage());
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
}

void loop() {
  // Ensure WiFi connection is active
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  else {
    displayTime();
  }
  delay(1000);
}

