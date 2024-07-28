#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include "config.h" 

// NTP-Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000);

// define the display
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(/*CS=*/ 10, /*DC=*/ 9, /*RST=*/ 8, /*BUSY=*/ 7));

void setup() {
  // initialize serial communication
  Serial.begin(115200);
  
  // set up the SPI bus
  SPI.begin(/*SCK=*/ 13, /*MISO=*/ -1, /*MOSI=*/ 11, /*SS=*/ 10);

  // initialize the display
  display.init(115200);

  // setup WiFi
  display.nextPage();
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(GxEPD_BLACK);
  display.println("Connecting to WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");

  }
  Serial.println("Connected to WiFi");
  display.println("Connected to WiFi");
  display.display();
  // NTP-Client initialise
  timeClient.begin();
}

void loop() {
  // fetch the time from the NTP-Server
  timeClient.update();

  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime); 

  ptm->tm_hour = (ptm->tm_hour + hourOffset) % 24;

  int day = ptm->tm_mday;
  int month = ptm->tm_mon + 1;
  int year = ptm->tm_year + 1900;
  int hour = ptm->tm_hour;
  int minute = ptm->tm_min;

  // format the date and time
  char dateBuffer[16];
  snprintf(dateBuffer, sizeof(dateBuffer), "%02d.%02d.%04d", day, month, year);

  char timeBuffer[16];
  snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d", hour, minute);

  // update the display
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.setTextColor(GxEPD_BLACK);
    display.println(String(dateBuffer) + "              " + String(timeBuffer) + " Uhr");
  } while (display.nextPage());

  delay(1000);
}

